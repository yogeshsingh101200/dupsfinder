# About
Dupsfinder is a duplicate file finder program which can search recursively through directories, disk drives and any other storage devices.

# Usage
- ** To compile: ** make dupsfinder
- ** To execute: ** ./dupsfinder \<directory> delete(optional)

# Benchmarks:
## Test system specs:
- Ryzen 5 2500U @2 Ghz(base) and 3.6 Ghz(boost), 4 cores
- 8 GB DDR4 Ram
- 1 TB 5400 RPM HDD of western digital

## Test Directories and drives

#### Directory 1: 4157 files taking 2 GB
##### Duplicates: 173 files taking 47.40 MB

* First run, time
  - Real: 18 secs
  - CPU: 2.1 sec

* Average of 10 test consecutive tests
  - Real: 0.823 secs
  - CPU: 0.8098 

#### Directory 2: 8633 files taking 8.07 GB
##### Duplicates: 2702 files taking 567.78 MB

* First run, time
  - Real: 51 secs
  - CPU: 6.5 secs

* Average of 10 consecutive tests
  - Real: 3.7 secs
  - CPU: 3.6 secs

#### Directory 3: 14901 files taking 105 GB
##### Duplicates: 4936 files taking 2.41 GB

* First run, time
  - Real: 4 min 30 secs
  - CPU: 15 secs

* Average of 10 consecutive tests
  - Real: 4 min 13 secs
  - CPU: 15 secs

# Algorithm
1. Loads files into hashtable on the basis of their sizes.
2. Compare every file to every other file on a same bucket at a time as follows
   - Compare their sizes.
   - If matched, compare their xxhash of first 2KB.
   - If matched, compare their sha256 hash.
   - If matched, push duplicate files to stack.
3. Print duplicate files by traversing stack.
4. If delete flag is used then pop files from stack and also delete them but leaving parent files.
5. Before exiting free all sorts of allocated memory.

#### Note:
###### I made this as my final project for CS50x.
