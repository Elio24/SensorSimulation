# Simulateur IoT - AP4A A26

FABRIZIO Elio - TP4B (semaine B).

Simulation séquentielle en C++17 d'un environnement de travail : température,
humidité, son et lumière. Aucun réseau, thread ou temps réel n'est nécessaire.

## Compiler et vérifier

Prérequis : CMake >= 3.20 et un compilateur C++17 avec `std::filesystem`
(par exemple GCC >= 9, MinGW récent ou Visual Studio 2022).

```sh
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Debug
cmake --build build-local --config Debug
ctest --test-dir build-local -C Debug --output-on-failure
```

Linux / WSL : `./build-local/SimulateurIOT`.
Windows avec MinGW : `.\build-local\SimulateurIOT.exe`.
Windows avec Visual Studio : `.\build-local\Debug\SimulateurIOT.exe`.
Avec MinGW, ajouter `-G "MinGW Makefiles"` à la première commande si nécessaire.
Utiliser un répertoire de compilation neuf lors d'un changement de compilateur.

Compilation GNU directe (depuis un shell Linux/WSL) :

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic src/*.cpp -Iinclude -o SimulateurIOT
./SimulateurIOT
```

Dans VS Code, ouvrir le dossier dans WSL avec l'extension C/C++ installée dans
WSL, puis utiliser la configuration de débogage fournie. Elle compile en Debug
dans `build-wsl` et lance le simulateur avec GDB.

## Exécution et journaux

La démonstration tourne sur six ticks avec cinq capteurs : deux températures
(périodes 2 et 3), un son entier, une humidité et une lumière booléenne.
Le scénario est volontairement chaud dans le bureau 2 (26 à 28 degC), sec
(35 à 45 %) et bruyant (72 à 82 dB), pour rendre les alertes visibles.
Résultats attendus : température 5 mesures agrégées (3 + 2), son 6,
humidité 2, lumière 6. Les valeurs aléatoires changent à chaque exécution.
Les valeurs initiales analogiques correspondent au milieu de la plage configurée
(arrondi vers zéro pour le son entier) ;
elles ne sont pas comptabilisées comme mesures avant `execute()`.

Chaque nouveau serveur crée un dossier unique `logs/simulation_<temps>_<suffixe>`.
Il contient les CSV par type, un journal texte et un inventaire obtenu par `<<`.
Colonnes CSV : `Tick;SensorID;Type;Valeur;Unite`. Le séparateur est le point-virgule
et le séparateur décimal est le point (à préciser lors d'un import dans un tableur).
Le tick d'une mesure est le nombre d'appels `update()` reçus par le capteur,
conservé entre plusieurs appels à `simulation()`. Dans la démonstration tous les
capteurs sont enregistrés avant le premier tick. Un appel direct à `execute()`
peut donc produire une mesure au tick 0.

Les statistiques sont regroupées par type. Le résultat booléen exprime la part
des mesures à 1, et non une durée physique d'allumage.

## Évolution progressive et alertes

Température, humidité et son évoluent depuis leur valeur précédente par une
marche aléatoire bornée. Variation maximale par mesure : 0,5 degC, 2 points
d'humidité et 5 dB. La valeur est limitée à la plage configurée. Ce pas dépend
du nombre de mesures, sans modèle de durée physique. La lumière reste un tirage
booléen indépendant. Une plage constante reste constante.

Le serveur n'active aucune règle implicitement. `main` configure les seuils de
démonstration : température > 25 degC, humidité < 45 %, son > 70 dB.
Ils sont pédagogiques et ne constituent pas des normes.

```cpp
server.setAlertThresholds("Temperature", std::nullopt, 25.0, "degC");
server.setAlertThresholds("Humidity", 45.0, std::nullopt, "%");
server.setAlertThresholds("Sound", std::nullopt, 70.0, "dB");
```

Une borne absente est représentée par `std::nullopt`. Les deux bornes peuvent
être définies. Une nouvelle configuration du même type remplace sa règle.
Les bornes doivent être finies, ordonnées, avec au moins une valeur présente.
Les mesures doivent employer l'unité configurée ; les booléens ne sont pas
acceptés pour ces règles numériques. Chaque mesure strictement hors seuil
déclenche une alerte, même si la précédente avait déjà déclenché une alerte.
Une égalité au seuil ne déclenche rien.

Les alertes sont affichées, comptées dans le bilan et sauvegardées dans
`alerts.csv` du dossier de session. Ce fichier est créé à la première alerte.
Colonnes : `Tick;SensorID;Type;Valeur;Unite;Direction;Seuil`.
Les mesures hors seuil restent dans les statistiques. Une erreur de sauvegarde
d'alerte arrête la réception avant mise à jour des statistiques ; le CSV de
mesures peut déjà contenir sa ligne. Il n'y a pas de transaction entre fichiers.
Les copies de serveur copient également les règles et le compteur d'alertes,
tout en conservant le chemin de journalisation partagé.

## Copies et durées de vie

- `main` possède le serveur et les capteurs ; le serveur leur survit.
- Les pointeurs de `Sensor` et `Scheduler` sont non propriétaires. Ils ne sont
  jamais libérés par ces classes ; les objets référencés doivent leur survivre.
- Copier un capteur attribue un nouvel identifiant et copie son état, son
  compteur temporel et sa référence au même serveur. Une affectation conserve
  l'identifiant de sa destination. Les identifiants positifs ne sont pas réutilisés
  au cours du processus, même après destruction. Les doublons explicites sont refusés.
- Copier un ordonnanceur copie son vecteur de références : ses capteurs restent
  partagés. Piloter ces copies fait donc avancer les mêmes objets.
- Copier un serveur copie ses statistiques par valeur et partage le chemin de
  journalisation. C'est une copie d'état, pas le début d'une nouvelle simulation.
  Créer un nouveau serveur nommé pour obtenir une session indépendante.

Une période invalide, des bornes inversées ou non finies et un identifiant
invalide déclenchent `std::invalid_argument`. Changer la période remet son
compteur interne à zéro. Une erreur d'écriture est propagée et la mesure
concernée n'est pas ajoutée aux statistiques. `main` affiche l'erreur et retourne 1.
Les ajouts répétés d'un même pointeur à l'ordonnanceur sont ignorés.

## Documents et rendu

Le rapport PDF est fourni séparément de l'archive du code. Le diagramme UML
est également disponible dans `docs/architecture.svg`.

L'archive de dépôt comprend les sources, les tests, CMake, ce guide et la
configuration VS Code. Elle exclut les exécutables, caches, journaux et outils
Python de fabrication du rapport. Les résultats de validation sont décrits dans
le rapport ; la validation Windows native reste à effectuer sur une machine équipée.
