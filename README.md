CleanShare: Don't OverShare
====================================================

CleanShare is a privacy-focused desktop app that automatically detects and redacts alcoholic beverages in your photos using a locally running AI model. Nothing is uploaded or shared and all processing happens entirely on your Windows 10/11 device.

Designed for social media, event photos, and large image galleries, CleanShare helps you quickly clean up pictures before posting so your personal or professional image stays clean.

CleanShare Installation Instructions (Windows 10/11)
====================================================

1. Download the Installer
-------------------------
Download the latest CleanShare installer from the project's GitHub Releases page:
"CleanShare-Setup.exe"

If you do not see a release available, contact the CleanShare team.


2. Run the Installer
--------------------
1. Double-click "CleanShare-Setup.exe"
2. If Windows shows a security prompt, click "Yes" to allow installation.
3. Follow the on-screen steps:
   - Choose an installation directory (the default is recommended)
   - Optionally create a desktop shortcut
4. Click "Install" to complete the setup.


3. Launch CleanShare
--------------------
After installation, you can run CleanShare from:

- The Start Menu → CleanShare
- The Desktop shortcut (if selected during installation)
- Directly via:
  C:\Program Files\CleanShare\CleanShare.exe


4. What the Installer Includes
------------------------------
The installer bundles all required components for CleanShare to run:

- CleanShare.exe (main application)
- Qt runtime DLLs
- OpenCV runtime DLLs
- ONNX Runtime libraries for AI inference
- Machine learning model:
    models\alcohol-detector-v1.onnx
- Additional plugin and platform dependencies

No separate installation of Qt, Python, or OpenCV is required.


5. Uninstalling CleanShare
---------------------------
To uninstall CleanShare:

1. Open the Start Menu
2. Search for "Add or remove programs"
3. Select "CleanShare" from the list
4. Click "Uninstall"

This removes all program files from the system.
