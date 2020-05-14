# Shape modeling
## Assignment 6

### Group Members

| Legi Number | First Name | Last Name | Email            | Github Username |
| ----------- | ---------- | --------- | ---------------- | --------------- |
| 16-944-530  | Viviane    | Yang      | vyang@ethz.ch    | vyangETH        |
| 17-948-191  | Daniel     | Sparber   | dsparber@ethz.ch | DanielSparber   |
| 15-941-222  | Sebastian  | Winberg   | winbergs@ethz.ch | winbergs        |

### Work distribution

- Project lead: Daniel
- Landmark selection: Sebastian
- Face alignment: Camilla (rigid), Jela (warping)
- PCA: Viviane
- Bonus: Daniel + whoever is finshed


### TODO

- [x] Viviane: Add template to repository
- [x] Viviane: add data


### Meetings

- 09.05.2020, 14:00
- TBA

### Project Structure

├── build/  
├── cmake  
│   └── FindLIBIGL.cmake  
├── CMakeLists.txt  
├── data/  
├── libigl/  
├── README.md  
├── src  
│   ├── main.cpp  
│   ├── example.cpp  
│   └── example.h  
└── test  
    └── example_main.cpp  



### Wiki

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