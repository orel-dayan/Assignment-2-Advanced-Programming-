# Assignment 2 for "Advanced Programming"
### Note on Performance
On one of our computers, 8 threads performed better, while on another computer, 4 threads yielded better results. The documentation is in the screenshots folder.
## Get Started


```bash

make 
time ./randomGenerator 10 10000000 | ./primeCounter 

our improvement 
time ./randomGenerator 10 10000000 | ./newCounter
for ram checker :
./randomGenerator 10 10000000 | valgrind --tool=massif ./newCounter


```

## Authors

- Orel Dayan
- Yuval Baror


