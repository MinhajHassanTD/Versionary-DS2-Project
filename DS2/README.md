# 📝 Versionary: An Image-Based Version Control System

### 📅 Project Duration: 4 Weeks (March 19 - April 13, 2025)
### 👥 Team Members:
1. **Azfar Ali** - aa08861  
2. **Minhaj ul Hassan** - mu08984  
3. **Muhammad Qasim Khan** - mk05539  
### 👨‍🏫 Instructor: Dr. Usman Arif  

---

## 🚀 Project Overview
Versionary is an image-based version control system designed to efficiently manage and track changes between image versions. The core of the system relies on **Merkle Trees** for secure and efficient version tracking. To further enhance processing, **grayscale conversion** reduces data complexity, and **Quadtrees** are used to break down images into smaller chunks. This combination allows Versionary to detect differences between versions, even in large images, making it a powerful tool for version control and visual quality assurance.

---

## 🗺️ Project Roadmap

### Week 1: Research and Project Setup (March 19 - March 25)
#### Tasks:
- **Research and Preparation:**  
  - Study **Merkle Trees**, **Quadtrees**, **SHA-256 hashing**, **OpenCV**, and **GUI libraries** (like Qt or ImGui).  
  - Understand how to implement **grayscale conversion**, **image chunking**, and **GUI design**.  
  - Review existing systems and version control techniques.  
- **Environment Setup:**  
  - Install **C++**, **OpenCV**, **Qt/ImGui**, and **Git** on all members’ systems.  
  - Set up a **GitHub repository** and project structure.  
  - Prepare a basic **project skeleton** with essential files.  
- **Team Responsibilities:**  
  - *Azfar:* Study **Quadtrees** and understand GUI libraries (Qt or ImGui).  
  - *Minhaj:* Study **Merkle Trees** and learn GUI basics.  
  - *Qasim:* Set up **OpenCV**, practice grayscale conversion, and explore GUI design.

---

### Week 2: Core Functionality Development (March 26 - April 1)
#### Tasks:
- **Merkle Tree Construction:**  
  - Implement **Merkle Tree structure** for storing image hashes.  
  - Develop efficient **SHA-256 hashing** for image chunks.  
  - Test hashing on small sample images.  
- **Grayscale Image Processing:**  
  - Implement **image reading** and **grayscale conversion** using OpenCV.  
  - Optimize the grayscale function to handle various formats.  
- **Quadtrees for Chunking:**  
  - Implement **Quadtree structure** for segmenting images into chunks.  
- **GUI Prototyping:**  
  - Develop a basic **GUI layout** with placeholders for commands and outputs.  
- **Team Responsibilities:**  
  - *Azfar:* Implement **Quadtrees** and integrate with image chunking.  
  - *Minhaj:* Implement **Merkle Trees** and hashing logic.  
  - *Qasim:* Develop **grayscale conversion** and start **GUI prototyping**.  

---

### Week 3: Version Comparison, GUI Integration, and CLI (April 2 - April 8)
#### Tasks:
- **Image Difference Detection:**  
  - Implement **difference detection** by comparing image hashes.  
  - Visualize changes between image versions.  
- **Command-Line Interface (CLI):**  
  - Develop a **CLI** for adding, committing, comparing, and viewing versions.  
- **GUI Integration:**  
  - Integrate **GUI with core functionality** (adding, comparing, and visualizing versions).  
- **Team Responsibilities:**  
  - *Azfar:* Enhance **Quadtrees** and integrate with difference detection.  
  - *Minhaj:* Finalize **Merkle Tree integration** and CLI commands.  
  - *Qasim:* Develop the **GUI** for displaying image comparisons and version history.  

---

### Week 4: Optimization, Testing, and Finalization (April 9 - April 13)
#### Tasks:
- **Optimization:**  
  - Improve **hash computation speed** and **memory efficiency**.  
  - Test with **high-resolution images** to ensure stability.  
- **Bug Fixes and Testing:**  
  - Perform **unit testing** and **integration testing**.  
  - Test for **edge cases**, including corrupted or unsupported files.  
- **Final GUI Touches:**  
  - Add tooltips, buttons, and visual enhancements to the GUI.  
- **Documentation and Presentation:**  
  - Write a comprehensive **user guide** and **project documentation**.  
  - Prepare a **demo presentation**.  
- **Team Responsibilities:**  
  - *Azfar:* Finalize **Quadtrees integration** and optimize chunk comparisons.  
  - *Minhaj:* Optimize **Merkle Tree operations** and enhance the CLI.  
  - *Qasim:* Polish the **GUI** and integrate visualization enhancements.

---

## ✅ Project Deliverables
- A complete **C++ application** implementing Versionary.  
- A **command-line interface** to add, commit, compare, and roll back image versions.  
- A **GUI** for interactive usage and visualization of version comparisons.  
- Visual representation of differences between image versions.  
- Detailed **documentation** and **user manual**.  
- **Presentation slides** for project demonstration.  

---

## 🔧 Tools and Technologies
- **Programming Language:** C++  
- **Image Processing:** OpenCV  
- **GUI Library:** Qt or ImGui  
- **Hashing:** SHA-256  
- **Data Structures:** Merkle Trees (primary), Quadtrees (supportive)  
- **Version Control:** GitHub for collaborative development  
- **Build System:** CMake or Makefile  

---

## 📌 Progress Tracking
- Use **GitHub Projects** and **Issues** to manage tasks and track progress.  
- Regular **team meetings** to discuss updates and challenges.  
- **Daily commits** to the repository for continuous progress tracking.  

---