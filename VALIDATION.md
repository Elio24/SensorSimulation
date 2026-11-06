# Validation de la version corrigée

Environnement : Linux via WSL, GNU C++ 15.2.0, C++17.

- Configuration CMake Debug : réussie.
- Compilation du programme et des tests : réussie, sans avertissement émis.
- CTest `simulator_contracts` : réussi.
- Démonstration sur six ticks : réussie, cinq mesures Temperature, six Sound,
  deux Humidity et six Light ; deux capteurs de température distincts.

Les vérifications détaillées sont dans `tests/test_simulator.cpp`.
Elles couvrent aussi les pas maximaux de la marche aléatoire, les seuils
hauts/bas, l'égalité aux seuils, les dépassements répétés, le CSV d'alertes,
la copie des règles et les erreurs d'écriture du journal d'alertes.
La compilation native Windows n'a pas été exécutée.
