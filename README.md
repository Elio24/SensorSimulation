# AP4A — Simulateur de capteurs

Projet de TP en C++17 réalisé par Elio Fabrizio dans le cadre de l’UE AP4A. Le programme simule les capteurs d’un bureau sur six étapes. Chaque capteur prend ses mesures à son propre rythme, puis le serveur les enregistre et calcule un bilan par type de mesure.

La démo utilise deux capteurs de température, un capteur de bruit, un capteur d’humidité et un capteur de lumière. Le scénario est volontairement chaud, sec et bruyant pour qu’on voie les alertes dès le premier lancement.

## Compiler et lancer

Il faut CMake 3.20 ou plus récent et un compilateur compatible C++17.

```sh
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Debug
cmake --build build-local --config Debug
ctest --test-dir build-local -C Debug --output-on-failure
```

Sur Linux ou WSL, lance `./build-local/SimulateurIOT`. Sous Windows, l’exécutable est dans `build-local` avec MinGW et dans `build-local/Debug` avec Visual Studio.

## Lire les résultats

Chaque exécution crée un dossier de session dans `logs/` avec les mesures en CSV, le journal et l’inventaire des capteurs. Lorsqu’une mesure dépasse un seuil configuré, elle apparaît aussi dans `alerts.csv`. Les valeurs varient à chaque lancement.

Le [diagramme UML](docs/architecture.svg) montre comment les capteurs, l’ordonnanceur et le serveur sont reliés. Le [rapport de TP](docs/Rapport_AP4A_TP4B_FABRIZIO_Elio.pdf) et les [notes de validation](VALIDATION.md) donnent les détails sur les choix d’implémentation et les tests.
