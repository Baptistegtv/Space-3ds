# Space — jeu d'exploration spatiale pour 3DS (homebrew .3dsx)

## Contrôles
- **Circle Pad** ou **Croix directionnelle** : piloter le vaisseau (les deux fonctionnent, en même temps si tu veux)
- **Touche Y** : activer/désactiver le pilote automatique (AUTO)
- **Écran tactile (bas)** : mini-map ; touche l'icône engrenage en haut à droite pour ouvrir les réglages (musique, couleur de l'interface)
- **START** : quitter le jeu

## Fonctionnement
- Le vaisseau se pose **automatiquement** dès qu'il est assez lent et assez proche d'une planète, et y plante un drapeau jaune.
- Chaque planète est générée procéduralement : type (rocheuse / désertique / océanique / géante gazeuse), taille, anneaux éventuels, masse, nombre de corps en orbite.
- Le HUD en bas de l'écran du haut affiche en temps réel : vitesse, puissance des propulseurs, coordonnées, infos sur l'étoile du système (nom, type, rayon, masse, nombre de corps), et le bouton AUTO.
- En hypervitesse, les étoiles s'étirent en traînées lumineuses.

## Compiler (via GitHub Actions, comme pour Abysse)
1. Crée un repo GitHub (ou réutilise un repo existant) et uploade **tout le contenu de ce dossier** tel quel (`source/`, `Makefile`, `.github/workflows/build.yml`).
2. Le workflow se déclenche automatiquement sur push vers `main`, ou manuellement via l'onglet **Actions > Build 3DSX > Run workflow**.
3. Une fois le build terminé, télécharge l'artefact **Space-3dsx** : il contient `Space.3dsx`.
4. Copie `Space.3dsx` dans `/3ds/` sur la carte SD de ta 3DS, lance le Homebrew Launcher.

## Notes techniques
- Pas d'assets externes (images/musique) : tout le rendu (planètes, étoiles, vaisseau) est généré avec des primitives citro2d (cercles, ellipses, lignes), donc aucun fichier à uploader dans `romfs/`.
- La "musique" dans le menu réglages est pour l'instant un sélecteur d'état (pas de vrai son joué) — dis-moi si tu veux que j'ajoute du vrai son avec `ndsp` (lecture de fichiers .wav/.ogg), ça demande d'ajouter des fichiers audio dans `romfs/`.
- Structure modulaire : `ship.c` (vaisseau), `planet.c` (génération + rendu planètes), `starfield.c` (fond étoilé), `hud.c` (panneau infos), `bottomscreen.c` (mini-map + réglages), `autopilot.c` (pilote auto).

## Si erreur de compilation
Colle-moi le message d'erreur exact du log GitHub Actions, je corrigerai direct (comme pour le bug `-mtp=soft` qu'on avait eu sur Abysse).
