### Code Acknowledgments
Template [OpenGL source code](https://github.com/TheCherno/OneHourParticleSystem/tree/master) was taken from TheCherno and used as a basis for this project to handle low-level OpenGL calls and window handling. The previously existing particle system implementation was mostly modified/removed for allow
the fluid/cloud particle simulation to be built on top of it.

My fluid/cloud simulation implementation can be found in ParticleSystem.cpp/.h, most of the rest of the code files are template/boilerplate code to get the simulation to run, including everything in the "OpenGL-Core" folder.

### Instructions
This code was developed on a Windows 11 64-bit system using Visual Studio, but it should run just fine in Windows 10. 

1. To run, clone the repo.
2. Open the folder and open "Simulation.sln" in Visual Studio.
3. Build the project and run it.

### References
The following research papers were used as the source of the implementation for this simulation project, ordered from most to least used:

[1]  M. J. Harris, W. V. Baxter, T. Scheuermann, and A. Lastra, “Simulation of cloud dynamics on graphics hardware,” in ACM SIGGRAPH 2005 Courses, New York, NY, USA: ACM, 2005, pp. 223-es. doi: 10.1145/1198555.1198793.

[2]  Fedkiw, J. Stam, and H. W. Jensen, “Visual simulation of smoke,” in Proceedings of the 28th annual Conference on Computer Graphics and Interactive Techniques, New York, NY, USA: ACM, 2001, pp. 15–22. doi: 10.1145/383259.383260.

[3] J. Stam, “Stable fluids,” in Proceedings of the 26th annual conference on Computer graphics and interactive techniques, New York, NY, USA: ACM Press/Addison-Wesley Publishing Co, 1999, pp. 121–128. doi: 10.1145/311535.311548.
