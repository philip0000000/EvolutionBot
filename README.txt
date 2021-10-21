==============================================================================================================
Evagents
Authors & Contributors: Julian Hershey, Andrej Karpathy, Casey Link, David Coleman 
Supporters: Eric Hvinden
License: BSD (see end of file)

Project website and attached forum: 
https://sites.google.com/view/evagentsproject/home

==============================================================================================================

WHAT IS EVAGENTS?
Evagents is a "natural" selection simulator. The program contains a world wherein agents can interact with that world and each other, reproduce, die, run around, eat, breed, communicate, and other things, using neural nets to calculate their responses to stimuli. Each agent has a huge number of features available to them that they can use in various ways to try and survive long enough to reproduce, extending their solution through time. Any time a baby is born its traits are copied from it's parent(s), but with chances of mutations. Thus a form of natural selection is applied to create populations of species that can best survive in the world. Competition and cooperation are important secondary characteristics that agents must develop, using their DRAWS Brains, a neural network with features of Dampening, Recurrence, Addition, Weighted, Sigmoid-outputs that takes inputs (in range 0-1) and feeds them to outputs (also 0-1) over time and in an initially random structure. As time passes, however, the brains can mutate into more efficient forms

==============================================================================================================

NOTE ABOUT VERSION NUMBERS:
Version numbers are in format x.xx.x

First decimal is the overall version and status of the program. 0= development, 1= final release version (fvr) 1, 2= frv 2, etc.

Second decimal is the release number. I expect this to get into double-digits in development status.

Third decimal is the pre-release number of the NEXT version. IE, v0.04.1 is the first pre-release of the next version, 0.05. This is reverse of how most others use these numbers, where they represent patches to the release version, but it's easier for me to make an update to the next release version than it is to make a new release version, so that's how that's going to be.

==============================================================================================================

BUILDING: (for users that want to edit the source code)

To compile Evagents you will need:
+ Microsoft Windows
+ MS SDK for Windows Server 2008 and .NET Framework 3.5 (https://www.microsoft.com/en-us/download/details.aspx?id=11310)
+ OpenGL* (http://www.opengl.org/resources/libraries/glut/)

It will use OpenMP to speed up everything, in case you have multi-core CPU, and OpenGL + GLUI to draw a user interface.

Take the provided .vcproj file and edit the files as you desire.
When finished, build the solution in either Debug or Release configuration, depending on your needs

*OpenGL is typically included in Windows distributions, so a manual DL is not usually required.
==============================================================================================================

QUESTIONS, COMMENTS, ISSUES are best posted on the google group, available here: https://sites.google.com/view/evagentsproject/discussion

==============================================================================================================

LICENSE CLAUSE:
Redistribution and use in source and binary forms are permitted provided that the above Authors list, project name, and this paragraph are duplicated with all distributions, and that any documentation, advertising materials, and other materials related to such distribution and use acknowledge that the software was developed by the Authors. The names of the Authors may not be used to endorse or promote products derived from this software without specific prior written permission. THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

==============================================================================================================
