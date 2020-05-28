# Shape modeling
## Assignment 6

### Resources 

- [Our Presentation](https://docs.google.com/presentation/d/1DIcviOEhOQk6m5dT0ryPQ2L86IimTuuQazykyQr2f8I/edit?usp=sharing)

- [Our meshes (polybox)](https://polybox.ethz.ch/index.php/s/TjBI4GNID1qZCtm?path=%2F)

- [Files, shared with all teams (polybox)](https://polybox.ethz.ch/index.php/s/ZfYXXfV5SR4sQoB)  (PW: eigenfaces)

### Group Members

| Legi Number | First Name | Last Name       | Email            | Github Username |
| ----------- | ---------- | ----------------| ---------------- | --------------- |
| 16-944-530  | Viviane    | Yang            | vyang@ethz.ch    | vyangETH        |
| 17-948-191  | Daniel     | Sparber         | dsparber@ethz.ch | DanielSparber   |
| 15-941-222  | Sebastian  | Winberg         | winbergs@ethz.ch | winbergs        |
| 18-949-685  | Camilla    | Casamento Tumeo | camillca@ethz.ch | ccasam          |
| 15-923-428  | Jela       | Kovacevic       | jelak@ethz.ch    | jelak           |

### Work distribution

- Camilla: Preprocessing, Rigid Alignment
- Daniel: Project Lead, Learning Based Approach (bonus)
- Jela: Warping
- Sebastian: Landmark Selection, User Interface
- Viviane: PCA

### Group Meetings

- 09.05.2020, 14:00
- 20.05.2020, 11:20
- 26.05.2020, 10:00
- 28.05.2020, 10:00

### Project Structure

`src/main.cpp` is the entry point for an application containing all pipline steps

`test/` contains main files for every pipline step individually

`learning/` contains the learning based approach (PyTorch)


### How To

#### Cloning the repository

```
git clone --recursive https://github.com/DanielSparber/sm-assignment6.git
```

#### Building

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j2
```

#### Add a new step to the pipeline
1. Add header and source file in `src`
2. Write your own test file in `test`
3. Add an executable for your step in CMakeLists.txt
4. After testing, add your files and functions in the pipeline main.cpp

