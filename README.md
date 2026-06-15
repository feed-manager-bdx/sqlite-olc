# sqlite-olc

Extension SQLite écrite en C qui ajoute des fonctions SQL de calcul de distance géographique à partir de codes Open Location Code (Plus Codes de Google) et de coordonnées latitude/longitude.

## Rôle dans l'écosystème

Ce dépôt fait partie du projet **RTP LVMH**, entité **Clients**.

Il s'agit d'une **librairie native** (module chargeable SQLite) qui enrichit le moteur SQLite avec trois fonctions de distance géodésique. Elle est destinée à être chargée comme extension dans une base SQLite afin de calculer, directement en SQL, des distances entre deux points exprimés soit en Open Location Code, soit en coordonnées géographiques.

L'extension s'appuie sur la bibliothèque officielle [google/open-location-code](https://github.com/google/open-location-code), intégrée comme sous-module Git (composant `c/`).

## Stack technique

| Élément | Détail |
| --- | --- |
| Langage | C |
| Cible de build | Bibliothèque partagée (`.so` sous Linux, `.dll` sous Windows) |
| Compilateur | `gcc` (Linux), `x86_64-w64-mingw32-gcc` (Windows, cross-compilation) |
| Build | `make` (`Makefile`) |
| Dépendances natives | En-têtes SQLite (`sqlite3ext.h`), bibliothèque mathématique (`math.h`) |
| Dépendance externe | `google/open-location-code` (sous-module Git, code C) |
| Tests / exemple | `test.py` (Python 3 via le module standard `sqlite3`) |
| Licence | GNU LGPL v3 |

## Structure du projet

```
.
├── sqlite-olc.c        # Code source de l'extension (fonctions SQL et calcul de distance)
├── Makefile            # Cibles de build : `all` (Linux/macOS) et `windows` (cross-compilation)
├── test.py             # Exemple d'utilisation et test manuel en Python
├── open-location-code/ # Sous-module Git : bibliothèque Google Open Location Code (code C utilisé via c/src/)
├── .gitmodules         # Déclaration du sous-module open-location-code
├── .gitignore          # Ignore l'artefact compilé sqlite-olc.so
├── LICENSE             # Licence GNU LGPL v3
└── README.md
```

### Fonctions SQL exposées

L'extension enregistre trois fonctions scalaires (voir `sqlite3_extension_init` dans `sqlite-olc.c`). Toutes retournent une distance en **mètres**, sous forme d'entier.

| Fonction | Arguments | Description |
| --- | --- | --- |
| `olc_distance(olc_a, olc_b)` | deux codes OLC (texte) | Distance entre les centres de deux Plus Codes. |
| `geo_distance(lat_a, lon_a, lat_b, lon_b)` | quatre flottants | Distance entre deux points en latitude/longitude. |
| `olc_geo_distance(olc, lat, lon)` | un code OLC (texte) + deux flottants | Distance entre le centre d'un Plus Code et un point en latitude/longitude. |

Détails de comportement :

- Le calcul utilise la formule de Haversine avec un rayon terrestre interpolé selon la latitude (entre rayon équatorial et rayon polaire).
- Si un argument est `NULL`, la fonction renvoie `NULL`.
- Si un argument n'a pas le type attendu (texte pour un OLC, flottant pour une coordonnée), la fonction lève l'erreur SQLite `Invalid parameter type`.
- Si un code OLC ne peut pas être décodé, la fonction lève l'erreur `Failed to parse OLC`.

## Prérequis

- `gcc` (ou un compilateur C compatible)
- `make`
- Les en-têtes de développement SQLite (paquet `libsqlite3-dev` sous Debian/Ubuntu, qui fournit `sqlite3ext.h`)
- Git (pour récupérer le sous-module `open-location-code`)
- Python 3 avec le module standard `sqlite3` (uniquement pour exécuter `test.py`)

## Installation & lancement (local)

1. Cloner le dépôt avec son sous-module :

   ```bash
   git clone --recurse-submodules <url-du-depot>
   cd sqlite-olc
   # ou, si le dépôt est déjà cloné sans le sous-module :
   git submodule update --init --recursive
   ```

2. Compiler l'extension :

   ```bash
   make
   ```

   Cette commande produit `sqlite-olc.so` à partir de `sqlite-olc.c` et du code C d'Open Location Code (`open-location-code/c/src/olc.c`).

3. Charger et utiliser l'extension (exemple Python) :

   ```python
   import sqlite3

   cnx = sqlite3.connect(":memory:")
   cnx.enable_load_extension(True)
   cnx.load_extension("./sqlite-olc")

   c = cnx.cursor()

   c.execute('SELECT olc_distance("9G7VPFJP+MX", "9G7VPFJQ+J2")')
   print(c.fetchone()[0], 'meters')  # 15 meters

   c.execute('SELECT geo_distance(44.9555555, -0.6912071, 46.2027364, 5.2294019)')
   print(c.fetchone()[0], 'meters')  # 481019 meters

   c.execute('SELECT olc_geo_distance("9G7VPFJP+MX", 46.2027364, 5.2294019)')
   print(c.fetchone()[0], 'meters')  # 2467728 meters

   c.close()
   ```

   Le script `test.py` fourni reprend exactement ces exemples ; on peut le lancer directement avec `python3 test.py` après avoir compilé l'extension.

   Pour un chargement depuis le client en ligne de commande `sqlite3` :

   ```sql
   .load ./sqlite-olc
   SELECT olc_distance('9G7VPFJP+MX', '9G7VPFJQ+J2');
   ```

### Build Windows (cross-compilation)

Une cible dédiée produit une DLL via MinGW :

```bash
make windows
```

Elle génère `sqlite-olc.dll` à l'aide de `x86_64-w64-mingw32-gcc`.

## Build via Docker

Le dépôt ne fournit pas de `Dockerfile` ni de `docker-compose`. Pour un build reproductible en conteneur, on peut s'appuyer sur une image disposant de `gcc`, `make` et des en-têtes SQLite, par exemple :

```bash
docker run --rm -v "$PWD":/src -w /src debian:stable-slim \
  sh -c "apt-get update && apt-get install -y gcc make libsqlite3-dev && make"
```

## Variables d'environnement

Aucune. Ce dépôt ne contient pas de fichier `.env` ni `.env.example` : il s'agit d'une extension native sans configuration runtime. Le comportement est entièrement déterminé par les arguments SQL passés aux fonctions.

## Déploiement

Cette librairie n'est pas un service : elle se « déploie » en distribuant l'artefact compilé (`sqlite-olc.so` ou `sqlite-olc.dll`) et en le chargeant dans le moteur SQLite de l'application consommatrice, via `load_extension()` (API) ou `.load` (CLI). Le chargement d'extensions doit être explicitement activé côté application (`enable_load_extension(True)` en Python).

L'artefact `.so` est volontairement ignoré par `.gitignore` et n'est pas versionné : il doit être recompilé sur la cible (ou produit par un pipeline de build) car il dépend de l'architecture et de l'ABI SQLite locale.

## Notes (incohérences et points d'attention)

- **Build Windows fragile** : la cible `windows` du `Makefile` référence un chemin absolu codé en dur `/usr/include/sqlite3ext.h`. De même, `sqlite-olc.c` inclut `"/usr/include/sqlite3ext.h"` sous `__WIN32`. Ces chemins en dur supposent un environnement de cross-compilation Linux précis et ne sont pas portables ; ils sont à adapter selon la machine de build.
- **Sous-module obligatoire** : sans `git submodule update --init`, le répertoire `open-location-code/` est vide et la compilation échoue (`olc.h` introuvable). Le `README.md` d'origine ne mentionnait pas cette étape.
- **Documentation d'origine minimale** : l'ancien `README.md` se limitait au build et à un exemple d'usage, sans décrire le rôle du dépôt, les fonctions exposées ni leur sémantique d'erreur. Ce point est corrigé ici.
- **Précision géodésique** : la fonction de rayon terrestre `get_earth_radius` mélange une approximation et n'applique pas la conversion degrés→radians à la latitude passée en argument ; les distances retournées sont des estimations à valider si une précision élevée est requise.
- **Sécurité** : aucun secret, clé ou identifiant n'est versionné dans ce dépôt. Le seul point d'attention reste générique : l'activation du chargement d'extensions SQLite (`enable_load_extension`) doit être maîtrisée côté application appelante, car elle permet de charger du code natif arbitraire.
