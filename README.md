So, from my notes I implemented the following obfuscation techniques:
1. Positional Offsets:
   Dynamic postional offsets to disrupt consistent stride length, and height analysis.
   Efficiency: Extremely high (basic math operations)
   Coverage: Stride pattern, height, global location.
3. Rotational Offsets:
   Same==[Positional Offsets]^^^
5. Jitter:
   Rotational jitter and noise: Adds small randomized changes to the orientation of trackers, like the feet, hands, and head. This is crucial for obfuscating specific joint angles and kinematics without running complex IK simulations.
   Efficiency: High (basic vector, quaternion math).
   Coverage: Joint angles, limb orientation, specific gestures.
7. Automated Blinks:
   A simple logic trigger.
   Efficiency: Trivial (sending a single data packet)
   Coverage: Facial micro expressions.

If you want the rest of my notes including some other things I was going to implement like, Thresholded axis glitches, per accessory physics and pendulum effects, corrective & dynamic blendshapes, physics simulation delay etc. just email me or try messaging me on my blog.
I may not have much information, but if you just want my insights, you can have it all~ 

mailto:sytreilia@gmail.com
https://sytreilia.blogspot.com/

Reciever UDP local host port: 39539
Transmitter UDP local host port: 39540
___________
|         |
|   WIN   |    <-39539
|_________|      39540->
    | |
-----------
|A|S|D|F|J|
|Z|X|C|V|B|
-----------
