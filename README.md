# Computational Graphics and Visualization

## Overview:
This project is a 3D rendering of a personal scene using OpenGL. The scene replicates special objects from a wedding and focuses on realistic textures, lighting, and object positioning. The project demonstrates technical skills in 3D programming, rendering, and problem-solving with OpenGL and C++.

## 3D Renderings:
| **Description**                                                                                               | **Image**                                                                                                                                          |
|---------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------|
| **Original Picture** <br /> <br /> This is the original photo that inspired the 3D scene, showcasing key wedding objects.                            | ![Wedding Overlay Picture](https://github.com/user-attachments/assets/f6e26690-31bf-4964-8294-62cf457f43ab)                                        |
| **3D Rendering (Top Down View)** <br /> <br /> A top-down view of the 3D scene that replicates the arrangement of wedding objects in the photo.      | ![image](https://github.com/user-attachments/assets/f6ab05b4-3b6c-497b-8f72-7c759ee7813f)                                                          |
| **3D Rendering (Front View)** <br /> <br /> The front orthographic view highlighting the arrangement and textures of the objects in the scene.       | ![Wedding Overlay 3D Rendering - Orth view](https://github.com/user-attachments/assets/fc212917-fd36-46f9-8ba3-659f992c6221)                        |
| **3D Rendering (Side View)** <br /> <br /> A side view of the 3D scene, showing the depth and dimensions of the objects from a different perspective.| ![Wedding Overlay 3D Rendering - side view](https://github.com/user-attachments/assets/98035f38-b160-4dcb-9a99-473567396b35)                        |


Technologies used: 
- **OpenGL** for 3D rendering
- **C++** as the programming language
- **Custom shaders** for lighting and material effects

## Architecture:
The project uses a combination of **OpenGL** for rendering 3D models and **custom shaders** to handle lighting and material properties.

- **Key Components:**
  - **OpenGL**: The core rendering library for drawing 3D objects.
  - **Custom Shaders**: Used to control lighting and texture applications.
  - **GLM**: A mathematics library for handling transformations and matrix calculations.

- **Architectural Decisions**:
  - **Modularity**: To keep the code clean, functions like `SetTextureUVScale()`, `DefineObjectMaterials()`, and `RenderScene()` were created to encapsulate texture scaling, material definitions, and rendering logic, respectively.
  - **3D Rendering Approach**: The choice of OpenGL allows fine control over vertex and fragment shaders, which made it easier to implement lighting, perspective shifts, and object transformations.

## Functionality:
- **Interaction**:
  The user can explore the 3D scene using both keyboard and mouse. Various camera perspectives can be toggled with specific keys (`U`, `I`, `O`, `P` for orthographic and perspective views), allowing navigation of the objects.

- **Code Refactoring Example**:
  Initially, textures were hard to scale, especially for small objects like the gold necklace. Refactoring the `SetTextureUVScale()` method helped scale textures dynamically based on object size. This improved the performance by reducing redundant texture bindings and increased code maintainability.

## Testing:
- **Approach**: 
  The project was tested through visual checks, ensuring that the 3D objects aligned with their real-life counterparts. Manual testing was conducted for keyboard navigation and camera control.
  
- **Challenges**:
  - **Lighting**: The hexagon’s lighting initially had issues where the top was brighter than the bottom, requiring adjustments to the normals and lighting calculations.
  - **Texture Scaling**: Adjusting texture UV scales on small objects like the necklace was particularly challenging.

## Reflection:
### How do I approach designing software?
I decided to replicate a significant moment from my wedding for my 3D scene project, which provided me with a clear direction and goal for my design. My ability to accurately depict real-world things in a digital setting has improved as a result of this project, particularly when working with intricate textures and shapes like the hexagon and gold necklace. I used an organized design method, segmenting the project into smaller, more manageable components and concentrating on resolving technical issues like lighting and texture mapping. In order to allow visitors to explore the scene from various angles, I also made sure that the user experience was easy to use by including basic navigation choices. I intend to continue using the strategies I employed, such as decomposing difficult issues and iteratively improving my design, in other projects. They enable me to maintain concentration and guarantee that the final work is both useful and significant.

### How do I approach developing programs?
I use iteration and problem-solving techniques a lot when creating applications. I had many difficulties when working on my 3D scene project, including resolving lighting problems and ensuring materials displayed properly. I used a modular strategy to address these, using custom functions like `DefineObjectMaterials()` and `LoadSceneTextures()` to keep my code structured and manageable. My success was mostly due to iteration; if anything didn't work out as planned, like the texture for the gold necklace, I would adjust the code and try again until I got the intended outcome. Throughout the project, my development methodology changed considerably, with each milestone highlighting how crucial it is to write clear, modular code and continuously improve it. I was able to overcome challenges and produce a polished end product because to this iterative procedure.

### How can computer science help me in reaching my goals?
I can now realize my technical and creative ideas thanks to computer science, especially with computational graphics and visualizations. I gained new abilities in lighting, texture mapping, and spatial thinking while working on my 3D scene project. These abilities can be used in a number of contexts, such as simulation programming and game creation. Even though I'm not actively pursuing game development, I think simulation programming has a lot of potential because it's so important to so many other sectors. I discovered how to control textures and lighting to produce realistic images, as well as how to manipulate objects in 3D space using XYZ coordinates. Through these experiences, I have improved my technical skills and gained a deeper understanding of how I might use computer science to further my education and career as a software engineer, with a focus on simulation programming.
