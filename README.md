## Build

You need a gcc, make, and sqlite3 headers to build this module.

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
# 15

c.execute("""SELECT geo_distance(44.9555555, -0.6912071, 46.2027364, 5.2294019)""")
print(c.fetchone()[0])
# 481069

c.execute("""SELECT olc_geo_distance("9G7VPFJP+MX", 46.2027364, 5.2294019)""")
print(c.fetchone()[0])
# 2465462

c.close()
```

