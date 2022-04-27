# Timer-Event-Generator
### Requirements
TIMER TYPE 1 - register_timer(const timepoint &tp, const timer_callback &cb): Type 1 timer’s callback function should be called exactly at timepoint. It is a one-shot timer.

TIMER TYPE 2 - register_timer(const millisecs &period, const timer_callback &cb): Type 2 timer’s callback function should be called every period_ms constantly. It is a periodic timer.

TIMER TYPE 3 - register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb): Type 3 timer’s callback function should be called if timepoint_ms is bigger than period_ms.

TIMER TYPE 4 - register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb): Type 4 timer’s callback function should be called if predicate returns true.
### Design
![resim](https://user-images.githubusercontent.com/44584158/165627356-5546f8b1-0b68-4a9a-863e-78a598311b32.png)

### Workflow

1.	Create timer object.
2.	Register timer member.
3.	Preschedule that member’s deadline and save it to scheduler table.
4.	Sort the table.
5.	If scheduler table is not empty, sleep the thread according to timer member that has the closest deadline.
6.	If timer member queue changes, wake up the thread and reschedule the table again.
7.	After the timer callback call, schedule the table and sort it again.

### Build and test

![resim](https://user-images.githubusercontent.com/44584158/165627589-33208b57-5086-4a9d-b34b-dd8bcc94ae58.png)

1.	Create a directory for keeping your build configuration:
mkdir -p out/build

2.	Create a directory for your source files.
mkdir src

3.	Compile & Run:
cmake -S ../../ -B . && make &&./timer
