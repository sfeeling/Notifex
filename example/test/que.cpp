#include <queue>
#include <iostream>

using namespace std;


int main()
{
    priority_queue<int> q;
    q.push(3);
    q.push(4);
    q.push(2);
    cout << q.top() << endl;
}