# Shape modeling
## Assignment 6

### Group Members

| Legi Number | First Name | Last Name | github username |
| ----------- | ---------- | --------- | --------------- |
| 16-944-530  | Viviane    | Yang      | vyangETH        |
|             |            |           |                 |
|             |            |           |                 |

### Work distribution

- Landmark selection: Sebastian
- Face alignment: Jela (warping), Camilla (rigid)
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