# Comparison of Maze Algorithms

## 1. Wall Follower
* Left Hand Rule: The solver will keep turning left when it can.
* Right Hand Rule: The solver will keep turning right when it can.
**Pros:** It doesn't require memory or map and its easy to implement.
**Cons:** It can easily get stuck in a loop , it doesnt gurantee the shortest path and  and it fails completely in non-simply-connected mazes.
  
## 2.Tremaux's Algorithm
* Solver marks each path it travels and it never enters a path marked twice it  prefers an unmarked path at an intersection, then takes a once marked path if no unmarked one exists, and otherwise backtracks this is essentially a physical implementation of Depth-First Search (DFS).
**Pros**: It doesnt require much memory space  and it solves any maze
**Cons**: It doesn't gurantee the shortest path and it may be hard to implement in a real life situation.

## 3.Flood Fill
* It assings a distance value variable to each cell showing how many steps would it take to achieve the goal. It finds the shortest path. It is dynamic and requires high memory usage.
**Pros**: It finds the shortest path (BFS).
**Cons**: It takes a lot of memory and a prior knowledge of the maze


## Comparison Table
  | Feature | Wall Follower | Tremaux's Algorithm | Flood Fill |
| :--- | :--- | :--- | :--- |
| **Handles Loops / Islands** | No | Yes | Yes |
| **Memory / Mapping** | None | Low (it marks per path) | High (full grid map) |
| **Finds Shortest Path** | No | No (finds a valid path) | Yes |
| **Best Use Case** | Simple mazes, blind navigation | Exploring unknown, complex mazes | Speedruns (for example: Micromouse) |
