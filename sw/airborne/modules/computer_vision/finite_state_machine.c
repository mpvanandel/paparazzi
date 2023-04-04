#include "finite_state_machine.h"

#include "modules/computer_vision/cv.h"
#include "modules/computer_vision/cv_opencvdemo.h"
#include "modules/computer_vision/opencv_example.h"

#include "modules/computer_vision/opencv_example.h"
#include "generated/airframe.h"
#include "state.h"
#include "modules/core/abi.h"
#include "math.h"
#include "generated/flight_plan.h"

#ifndef OPENCVDEMO_FPS
#define OPENCVDEMO_FPS 0       ///< Default FPS (zero means run at camera fps)
#endif

#define FLOWMIDDLE_DIV_LOWER 0.80f
#define FLOWMIDDLE_DIV_UPPER 1.4f
#define FLOWMIDDLE_DIV_2_STEPS_LOWER 0.5f
#define FLOWMIDDLE_DIV_2_STEPS_UPPER 2.0f
#define OBS_SIDES_TH 1.3f

#define COUNTER_TURN_1_MAX 3
#define COUNTER_TURN_2_MAX 9
#define HEADING_INC_LEFT_RIGHT 45.f
#define HEADING_INC_MIDDLE 60.f
#define HEADING_INC_OUT_OF_BOUND 90.f
#define MOVEDISTANCE 1.2f

#define LOG(x) fprintf(stderr, "LOG: %s:%d %s %lu \n", __FILE__, __LINE__, x, clock());

enum navigation_state_t {
  SAFE,
  JUST_TURNED,
  OBSTACLE_LEFT,
  OBSTACLE_RIGHT,
  OBSTACLE_MIDDLE,
  OUT_OF_BOUNDS,
};

struct OpticalFlow
{
  float left = 0.f;
  float right = 0.f;
  float middle = 0.f;
  float middle_prev = 0.f;
  float middle_prev_prev = 0.f;
  float left_right_ratio = 0.f;
  float middle_divergence = 0.f;
  float middle_divergence_prev = 0.f;
}

static uint8_t moveWaypointForward(uint8_t waypoint, float distanceMeters);
static uint8_t calculateForwards(struct EnuCoor_i *new_coor, float distanceMeters);
static uint8_t moveWaypoint(uint8_t waypoint, struct EnuCoor_i *new_coor);
static uint8_t increase_nav_heading(float incrementDegrees);

OpticalFlow flow;
enum navigation_state_t navigation_state = SAFE;
int counter_turn_1 = 0;
int counter_turn_2 = 0;

void run_fsm(float flow_left, float flow_middle, float flow_right)
{
  flow.middle_prev_prev = flow.middle_prev
  flow.middle_prev = flow.middle
  flow.left = flow_left;
  flow.middle = flow_middle;
  flow.right = flow_right; 
  flow.left_right_ratio = flow.left/flow.right;
  flow.middle_divergence = flow.middle/flow.middle_prev;
  flow.middle_divergence_prev = flow.middle/flow.middle_prev_prev;

   switch (navigation_state){
      case SAFE:
        state_safe();
      case JUST_TURNED:
        state_just_turned();
      case OBSTACLE_LEFT:
        state_obs_left();
      case OBSTACLE_MIDDLE:
        state_obs_middle();
      case OBSTACLE_RIGHT:
        state_obs_right();
      case OUT_OF_BOUNDS:
        state_out_of_bounds();
}

void state_safe(void)
{
  moveWaypointForward(WP_TRAJECTORY, MOVEDISTANCE);
      if (!InsideObstacleZone(WaypointX(WP_TRAJECTORY),WaypointY(WP_TRAJECTORY))){
        printf("OUT OF BOUNDS \n");
        navigation_state = OUT_OF_BOUNDS;
        return;  

      } 
      else if (flow.left_right_ratio < 1/OBS_SIDES_TH){
        printf("Obstacle RIGHT \n");
        navigation_state = OBSTACLE_RIGHT;
        break; 
        
      } else if (flow.left_right_ratio > OBS_SIDES_TH){
        printf("Obstacle LEFT \n");
        navigation_state = OBSTACLE_LEFT;
        break; 
        
      } 
        else if (flow.middle_divergence > FLOWMIDDLE_DIV_UPPER){
        printf("Obstacle MIDDLE \n");
        navigation_state = OBSTACLE_MIDDLE;
        break; 
        
      } else if (flow.middle_divergence < FLOWMIDDLE_DIV_LOWER){
        printf("Obstacle MIDDLE \n");
        navigation_state = OBSTACLE_MIDDLE;
        break; 
      } 
        else if (flow.middle_divergence_prev < FLOWMIDDLE_DIV_2_STEPS_LOWER){
        printf("Obstacle MIDDLE \n");
        navigation_state = OBSTACLE_MIDDLE;
        break; 
      } 
      else if (flow.middle_divergence_prev > FLOWMIDDLE_DIV_2_STEPS_UPPER){
        printf("Obstacle MIDDLE \n");
        navigation_state = OBSTACLE_MIDDLE;
        break; 
      }      
      else {
        moveWaypointForward(WP_GOAL, 0.5f * MOVEDISTANCE);
        printf("NO OBSTACLE \n");
      }
}

void state_obs_right(void)
{
  waypoint_move_here_2d(WP_GOAL);
  waypoint_move_here_2d(WP_TRAJECTORY);
  increase_nav_heading(-HEADING_INC_LEFT_RIGHT);
  navigation_state = JUST_TURNED;
}

void state_obs_left(void)
{
  waypoint_move_here_2d(WP_GOAL);
  waypoint_move_here_2d(WP_TRAJECTORY);
  increase_nav_heading(HEADING_INC_LEFT_RIGHT);
  navigation_state = JUST_TURNED;
}

void state_obs_middle(void)
{
  waypoint_move_here_2d(WP_GOAL);
  waypoint_move_here_2d(WP_TRAJECTORY);
  increase_nav_heading(HEADING_INC_MIDDLE);
  navigation_state = JUST_TURNED; 
}

void state_out_of_bounds(void)
{
  LOG("Out of bounds")
  waypoint_move_here_2d(WP_GOAL);
  waypoint_move_here_2d(WP_TRAJECTORY);
  increase_nav_heading(HEADING_INC_OUT_OF_BOUND);
  moveWaypointForward(WP_TRAJECTORY, MOVEDISTANCE);
  if (!InsideObstacleZone(WaypointX(WP_TRAJECTORY),WaypointY(WP_TRAJECTORY))){
    navigation_state = OUT_OF_BOUNDS; 
    }
  else {
    LOG("NOT OUT OF BOUNDS ANYMORE")
    moveWaypointForward(WP_GOAL, MOVEDISTANCE);
    navigation_state = JUST_TURNED; 
    counter_turn_1 = COUNTER_TURN_1_MAX;
  }
}

void state_just_turned(void)
{
  moveWaypointForward(WP_TRAJECTORY, 0.5 * MOVEDISTANCE);
  moveWaypointForward(WP_GOAL, 0.5 * MOVEDISTANCE);
  counter_turn_1++;
  if (counter_turn_1 < COUNTER_TURN_1_MAX){
    return;
  }

  counter_turn_2++;
  if (!InsideObstacleZone(WaypointX(WP_TRAJECTORY),WaypointY(WP_TRAJECTORY))){
    navigation_state = OUT_OF_BOUNDS;
    reset_turn_counters();
    return;
        
  
  } else if (flow.left_right_ratio < 1/OBS_SIDES_TH){ // added this
    navigation_state = OBSTACLE_RIGHT;
    reset_turn_counters();
    return;
         
  } else if (flow.left_right_ratio > OBS_SIDES_TH){ 
    navigation_state = OBSTACLE_LEFT;
    reset_turn_counters();
    return;
  } 
  if (counter_turn_2 == COUNTER_TURN_2_MAX){
    navigation_state = SAFE;
    reset_turn_counters();
  }
}

void reset_turn_counters(void)
{
  counter_turn_1 = 0;
  counter_turn_2 = 0;
}