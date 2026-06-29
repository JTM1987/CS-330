# CS-330 — Computational Graphics and Visualization

**Jesse Martin** | Southern New Hampshire University | June 2026

Portfolio repository for SNHU **CS-330: Computational Graphics and Visualization**. This repo showcases my **Module 7 final project**: a fully lit, textured 3D desk scene built in C++, OpenGL, GLEW, and GLFW.

---

## Portfolio artifacts

| Artifact | Location |
|----------|----------|
| **3D Scene** (source, Visual Studio project, executable) | [`7-1-Final-Project/`](7-1-Final-Project/) |
| **Design Decisions document** | [`docs/7-1 Final Project Design Decisions JMartin.docx`](docs/7-1%20Final%20Project%20Design%20Decisions%20JMartin.docx) |
| **Scene screenshot** | [`7-1-Final-Project/finalproject.png`](7-1-Final-Project/finalproject.png) |

### Running the 3D scene locally

1. Open `7-1-Final-Project/7-1_FinalProjectMilestones.sln` in **Visual Studio 2022**.
2. Set configuration to **Debug | x86**.
3. The project expects course textures at `../../Utilities/textures/` relative to the executable (standard **CS330Content** layout). Alternatively, run the included `Debug/7-1_FinalProjectMilestones.exe` if you have CS330Content installed at `C:\CS330Content\`.

**Controls:** WASD + QE to move, mouse to look, scroll to adjust speed, **O** / **P** for orthographic / perspective projection, **Escape** to exit.

---

## Module 8 journal reflection

### Designing software

I approach software design by breaking a complex problem into smaller, verifiable pieces. For this course that meant starting with a **project proposal** and reference photograph, listing every object in the scene, and deciding which real-world items could be built from OpenGL **primitives** (box, cylinder, sphere, torus, plane, cone, tapered cylinder). I chose a front-facing desk workspace (Module One Image 5) because it had clear depth and a manageable object count. Complex shapes were simplified on purpose—the monitor became several boxes, the mug a tapered cylinder plus torus handle, and decorative items from my original flat-lay idea were dropped so I could finish a polished scene on time.

That process taught me **low-poly scene design**: thinking in terms of layout on the X, Y, and Z axes, grouping materials by object, and placing lights so the whole desk reads clearly from the camera. I followed the **milestone sequence** the course provided—geometry and transforms first, then camera interactivity, textures, lighting, and final integration—rather than trying to perfect everything at once. Those tactics apply directly to future work: block out structure, add interaction, then layer presentation details.

### Developing programs

On the development side I built on SNHU starter projects instead of rewriting from scratch. Most of my work lives in **SceneManager.cpp** (objects, materials, textures, lights) and **ViewManager.cpp** (camera, input, projection). I split drawing into focused methods such as `RenderMonitor`, `RenderMug`, and `SetupSceneLights` so the main loop stays readable. Each milestone added one capability—WASD movement in Module 4, texture loading in Module 5, Phong lighting in Module 6—and the final project combined and tuned them (camera angle, light intensity, material values to reduce blowout).

Iteration was constant: compile, run, adjust transforms, repeat. Over the term my code became more **modular and commented**, with consistent file headers and contributor notes. Debugging in Visual Studio and keeping builds on **Debug | x86** became routine. Module 8’s 2D collision assignment was a useful contrast—it reinforced state changes and physics in a simpler environment before returning to portfolio work.

### Goals and pathways

Computer science—and this graphics course specifically—helps me build a **portfolio employers can run and inspect**. Understanding the rendering pipeline, transformation matrices, shaders, and user input connects to game development, simulation, medical visualization, CAD, and interactive tooling. For further education, this course is a foundation for advanced graphics, HCI, or game-engine coursework. Professionally, the ability to translate a client brief (a 2D reference image) into an interactive 3D prototype mirrors real studio workflows and demonstrates both **technical skill** and **design judgment**.

---

## Course projects (overview)

| Module | Project |
|--------|---------|
| 2–3 | OpenGL setup, 3D shapes, milestone scene layout |
| 4 | Camera movement and scene interactivity |
| 5 | Texturing 3D objects |
| 6 | Phong lighting and lit complex objects |
| **7** | **Final 3D desk scene** *(portfolio centerpiece)* |
| 8 | 2D collision animation + GitHub portfolio journal |

---

## License

This repository is licensed under the [GPL-3.0 License](LICENSE).

Course starter code © SNHU / Brian Battersby. Student extensions and scene work © Jesse Martin.
