# Quantum-graph-simulation/v3-hashmap/test

## Requirement

 - **_libboost-dev_** pour des fonction de hashage.
 - **_libtbb-dev_** (partie de **openapi** d'intel) pour les `concurrent_unordered_multimap` qui sont des `unordered_multimap` thread-safe.
 - **_libomp-dev_** pour profiter du parallelisme.

## Compilation

Pour compiler un ficher **_fichier_.cpp** il suffit d'utiliser `make fichier`.

### Flags

Des _flags_ peuvent etre ajouté avant les autres targets de compilation:

 - _test_ : Ajoute une etape de test à _quantum\_iteration_.
 - _verbose_: ajoute certain print de debugage.

### Programmes:

 - _classical\_iteration_: Itere 6 fois 1M d'iteration et print les tailles (peut etre utilisé avec [../grapher/1d_grah.py](../grapher/1d_grah.py) pour generer un graphique d'evolution de la taille).
 - _graph\_name\_test_ et _node\_test_: Fait un simple test des type **graph_name** et **node** respectivement.
 - _graph\_test_ et _state\_test_: Permet de tester la dynamique (classique et quantique respectivement) sur un graphs aléatoire ou entré dans la console.
 - _calssical\_injectivity\_test_ : Verifie la bijectivité de la dynamique classique.
 - _quantum\_iteration_: Itere le cas quantique.
 - _quantum\_injectivity\_test_: opere 4 iterations du cas quantique puis autant d'iterations inverse pour verifier que l'on retourne bien à l'etat de depart.