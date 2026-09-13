# CW Ladder

CW Ladder is a Qt6/C++ desktop application that helps prepare a log for
the **EUCW Snake & Ladder** CW contest.

It lets you:
- import an ADIF file and automatically filter out the QSOs that qualify
  for the contest (duration ≥ 5 minutes, name logged, callsign in a
  European country);
- look up a QSO's QRA locator from its QTH (OpenStreetMap/Nominatim
  geocoding) or from HamQTH.com;
- compute the Snake & Ladder score of every exportable QSO and plot it on
  a map (green = Ladder, red = Snake, blue = neither);
- export the finished log back to ADIF.

The interface is available in English (default), French, Spanish,
Italian, German, Polish, Hungarian, Portuguese, Dutch, Finnish,
Norwegian and Swedish (Settings → Language). An in-app user guide is
available from Help → User Guide.

## Building on Linux

Requirements: Qt 6 (Core, Gui, Widgets, Network, LinguistTools) and
CMake ≥ 3.19.

```bash
# Debian / Ubuntu / Linux Mint: install the Qt6 dependencies if needed
sudo apt update
sudo apt install build-essential cmake ninja-build \
    qt6-base-dev qt6-base-dev-tools qt6-l10n-tools \
    libgl1-mesa-dev

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
./CWLadder
```

No separate install step is required to run it locally: the country
table, translations and icon are all embedded in the executable (see
below), so the single binary produced by the build is self-contained.

## Releases (Windows)

Pre-built 64-bit Windows binaries are published on the project's GitHub
Releases page as a ready-to-run `.zip` (built with Qt Creator/CMake and
packaged with `windeployqt`) — no compiler or Qt installation needed on
the target machine. Grab the latest `CWLadder-windows-x64.zip` from
Releases, unzip it anywhere, and run `CWLadder.exe`.

Qt6 no longer ships an official 32-bit build for Windows, so only 64-bit
Windows is provided; this covers virtually every machine still capable
of running Windows 10/11 today.

## Project layout

```
src/
  main.cpp                 entry point, language setup
  mainwindow.*             main window
  editlocdialog.*          "Get Locator" dialog
  resultsdialog.*          "Results" dialog
  settingsdialog.*         settings (HamQTH credentials, language, home locator)
  helpdialog.*             in-app user guide (Help menu)
  adifparser.*, adifwriter.*   ADIF import/export
  countrytable.*           callsign -> country table (ctyEUCW.csv + regex)
  geoloc.*                 Nominatim geocoding + Maidenhead conversions
  hamqth.*                 HamQTH.com API client
  scoring.*                Snake & Ladder scoring
  mapserver.*              local HTTP server for the Leaflet map
  appsettings.*            QSettings wrapper (no Windows registry, see below)
translations/              the 11 .ts files (English is the source language, built into the code)
resources/ctyEUCW.csv      European callsign/country table
resources/appicon_*.png, appicon.ico, app.rc   application icon
```

## Resources embedded in the binary

The European callsign table (`resources/ctyEUCW.csv`) and the 11
translation files are compiled into Qt resources and embedded in the
executable: a single binary is enough, nothing else to deploy alongside
it.

The icon (snake + ladder logo) is embedded two complementary ways: the
`resources/appicon_*.png` files (16 to 256px) are compiled into a Qt
resource and set at startup via `QApplication::setWindowIcon()` (window
and taskbar icon on every platform); on Windows only, `resources/app.rc`
additionally embeds `resources/appicon.ico` directly into the
executable, so the icon also shows up in Explorer, the taskbar and jump
lists before any window has even opened.

Settings (HamQTH credentials, home locator, language) are stored as a
plain `.ini` file rather than the Windows registry — see
`AppSettings::settingsFilePath()` in `appsettings.cpp` for the exact
portable/fallback location logic, and the Settings dialog, which
displays the file actually in use.

## Screeshots
<img width="1102" height="782" alt="Image" src="https://github.com/user-attachments/assets/8f8b7f88-a94d-4b63-a4c7-dd345626bed9" />

<img width="1098" height="776" alt="Image" src="https://github.com/user-attachments/assets/50b322ad-6bb0-4933-9eab-862062f99efe" />

<img width="1227" height="823" alt="Image" src="https://github.com/user-attachments/assets/b1d0b110-cbfd-4887-ba46-8e1699977221" />
