/*multicore cpu
8 temperature sensors
sensors check at regular intervals, shared memory space

report at the end of every hour, which shows the 
5 highest temps of the hour, 
5 lowest of the hour, 
and the 10 min interval when the largest difference was observed

careful handling, no delays or missing intervals

temp readings are done every minute
the sensors generate a random number from -100 to 70F

*/