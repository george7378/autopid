# autopid
AutoPID is a small demonstrator program for a simple autonomous spacecraft autopilot. The autopilot consists only of layered PID (proportional-input-derivative) controllers which monitor and respond to factors such as speed, bank angle and distance to target.

In single-point mode (the default mode), the target point can be moved around using the **Arrow** keys. The spacecraft will hopefully follow the target wherever you put it. Pressing **Space** will enter multi-point mode. Here, there are four target points and the spacecraft will cycle between them indefinitely. Press *R* to randomise the target locations.

Pressing **Enter** begins a frame dump. This saves every frame as a JPEG file into the 'framecapture' folder within the program's directory. This is a slow process and I only included it so that I could make a good frame rate demo video.
