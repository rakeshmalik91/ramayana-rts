# Ramayana - RTS Game

<div align="center">
	<img src="https://cdn.cloudflare.steamstatic.com/steam/apps/371790/header.jpg" alt="Ramayana RTS Logo" width="300"/>
	<br/>
	<b>Real-Time Strategy in the Age of Mythology</b>
</div>
<br/>

**Ramayana** is a Real-Time Strategy (RTS) game inspired by the Indian epic *Ramayana*. Command the armies of Rama or Ravana, manage resources, build bases, and lead legendary heroes into battle.

> ⚠️ **Note**: This project is legacy software (approx. 2015 era). Dependencies and build systems may require older environments (e.g., Visual Studio 2012).

## 🏙️ Stores
- **[Steam](https://store.steampowered.com/app/371790/Ramayana)** (Released 2015)
- **[IndieDB](https://www.indiedb.com/games/ramayana)** (Released 2015)

## 🎬 Trailer
<p align="center">
	<a href="https://www.youtube.com/watch?v=k43z6TWNgqM">
		<img src="https://img.youtube.com/vi/k43z6TWNgqM/0.jpg" alt="Ramayana RTS Cinematic Trailer" width="500"/>
	</a>
</p>

## 🎮 Gameplay
<p align="center">
	<a href="https://www.youtube.com/watch?v=eeJIrkLibdw">
		<img src="https://img.youtube.com/vi/eeJIrkLibdw/0.jpg" alt="Ramayana RTS Gameplay Video" width="500"/>
	</a>
</p>

<p align="center">
	<img src="https://cdn.cloudflare.steamstatic.com/steam/apps/371790/ss_72b1c64bf4e234f47d86bdb8a8ad10804cd074f6.1920x1080.jpg" width="45%" style="margin: 5px;"/>
	<img src="https://cdn.cloudflare.steamstatic.com/steam/apps/371790/ss_fc4c9094a7bec6c02c10898cb5305c44caed2dcd.600x338.jpg" width="45%" style="margin: 5px;"/>
</p>

## ✨ Features
- **Campaign Mode**: Follow the story of the Ramayana through scripted scenarios.
- **Skirmish Mode**: Custom battles with adjustable AI difficulty ranges.
- **Resource Management**: Gather **Food**, **Wood**, **Stone**, and **Metal** to build your empire.
- **Hero Units**: Control legendary characters like Rama, Lakshmana, Hanuman, and Ravana, each with special abilities.
- **Map Editor**: Create and edit your own custom maps.
- **Multiplayer**: LAN/Network support for multiplayer battles.

## 🕹️ Controls
Standard RTS control scheme:
| Action | Input |
| :--- | :--- |
| **Select Unit** | `Left Click` |
| **Move / Attack** | `Right Click` |
| **Box Select** | `Click & Drag` |
| **Camera Pan** | `Edge Scroll` or `Arrow Keys` |

## 🛠️ Technical Details

The engine is a custom C++ build using OpenGL for rendering and SDL for windowing/input.

### Legacy Build Dependencies
- **IDE**: Visual Studio 2012
- **Graphics**: OpenGL (GLEW 1.9.0, FreeGLUT)
- **Audio/Input**: SDL 1.2.15, SDL Mixer, SDL Net
- **Video**: OpenCV 2.4.9, FFmpeg 0.5.1
- **Other**: RapidXML 1.13, Dirent 1.20.1

### How to Build (Hypothetical)
1. Install **Visual Studio 2012**.
2. Ensure all external library headers and `.lib` files are correctly linked in the project properties (check `Ramayana.vcxproj`).
3. Open `Ramayana.sln`.
4. Build in **Release** or **Debug** mode (Win32).

---
*Developed by Rakesh Malik*
