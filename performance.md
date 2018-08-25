# Profiling information

This is a summary of data collected in order to understand the performance impact
of the 'bob' APIs.

Extra info:
- System: macOS 10.13.6 64bit. Node.js 10.9.0.
- Hardware: 2.6 GHz Intel Core i7, 8 GB 1600 MHz DDR3, MBP 2012 SSD.
- File: Unity 2018.11f1's `.app`, zipped. ~1.04 GB.

## Profiles

### `streams3-test.js`

[Profile using 1.04GB file](profiles/streams3-prof)

Output from `/usr/bin/time`:
```
      330.43 real         7.29 user        13.94 sys
```


### `index.js` _(bob socket)_

[Profile using 1.04GB file](profiles/bob-prof)

Output from `/usr/bin/time`:
```
       18.49 real         0.46 user         0.83 sys
```
