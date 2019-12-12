# EC504 final project

Jiaming Yu  
U72316560  
jiamingy@bu.edu  


The details of the project are in the file named EC504 project report.  

The project implements three versions of k-d tree to design the nearest state/county finder. Based on the tree data structure built by more than 2 million reference points, User can query the k nearest neighbors and find the state and county. THe time complexity of query is O(logn) while after using some tricks, it takes about 5e-5 to query.  

You can focus on the kdleaves.cpp.  

The codes are also saved on SCC:  /projectnb/alg504/jiamingy/project   
(1)Split by median and store data in leaves  
Code is in kdleaves.cpp, make file is named makekdleaves and the output is named kdleaves  
(2)Split by median and store data in node and leaf  
Code is in kdmedian.cpp, make file is named makekdmedian and the output is named kdmedian  
(3)Randomly insert and store data in node and leaf  
Code is in main.cpp, make file is named Makefile and the output is named KNN.  
 
