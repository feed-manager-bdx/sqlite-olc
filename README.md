## Build

You need a C compiler (gcc or clang for instance), make, and sqlite3 headers to build this module.

```
make
```

## Usage

```python
import sqlite3

cnx = sqlite3.connect(":memory:")
cnx.enable_load_extension(True)
cnx.load_extension("./sqlite-olc")

c = cnx.cursor()
c.execute("""SELECT olc_distance("9G7VPFJP+MX", "9G7VPFJQ+J2")""")
print(c.fetchone()[0])
>>> 15
c.close()
```


