#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/unistd.h>
#include <time.h> 
#include <signal.h> 


int Rfib(long long int  n){
    if(n==0 || n==1){
        return n;
    }
    else{
        return (Rfib(n-1)%2021+Rfib(n-2)%2021)%2021;
    }
}


struct matrix{
    unsigned int ri;
    unsigned int ci;
    long long int value;
    struct matrix *right;
    struct matrix *down;
};

// This create function creates 2 sparse matrices

void create(unsigned int n,struct matrix *col_head1,struct matrix *col_head2,struct matrix *row_head1,struct matrix *row_head2,struct matrix *head1,struct matrix *head2){
    
    //declaring and initialising all the required temporary variables and pointers to create the matrix

    int a,pa=0,d=0;
    unsigned int r,c,pr=0,i;
    long long int v;
    struct matrix *t1,*t2,*p1,*p2,*r1,*r2,*temp;
    
    r1=row_head1;
    r2=row_head2;
    t1=head1;
    t2=head2;
    p1=col_head1;
    p2=col_head2;
    
    //creating column headers for matrix 1 and matrix 2 and linking them to each other using the *right pointer
    //present in the structure and the *down pointer is initialised to NULL

    struct matrix* (m1[n+1]);
    struct matrix* (m2[n+1]);

    for(i=0;i<=n;i++){
        m1[i]=p1;
        m2[i]=p2;

        p1->right=(struct matrix *)malloc(sizeof(struct matrix));
        p1=p1->right;
        p1->right=NULL;
        p1->down=NULL;
        p1->ci=i;

        p2->right=(struct matrix *)malloc(sizeof(struct matrix));
        p2=p2->right;
        p2->right=NULL;
        p2->down=NULL;
        p2->ci=i;

    }

    //reading the two given matrices
    int id = 1;
    while(id<1000){
        id++;
        if(id<500){
            a = 1;
        }
        else{
            a = 2;
        }
        r = id;
        c = n - id;
        v = n/id + 1;  
        //scanf("%u %u %lld",&r,&c,&v);
        //for 1st matrix

        if(a==1){
            t1->right=(struct matrix *)malloc(sizeof(struct matrix));   //storing the values of the non-zero element
            t1=t1->right;
            t1->ri=r;
            t1->ci=c;
            t1->value=v;
            t1->down=NULL;
            t1->right=NULL;
        
            if(pr==0 || pr==r){
                if(d==0){           //checks if the input element is the first element of the first row
                    d=1;
                    if(pr==0){          
                        r1->value=r;
                        //create a row head node for each separate row
                        r1->right=(struct matrix *)malloc(sizeof(struct matrix));   
                        r1->right=t1;
                        r1->down=NULL;
                    }
                }

                //linking the non-zero element to it's corresponding column
                (m1[c])->down=t1;
                m1[c]=(m1[c])->down;
                (m1[c])->down=NULL;
            }
            else{
                /*linking all the non-zero elements linearly 
                and connecting row headers and pointing row headers to first element of row*/ 
                d=0;
                r1->down=(struct matrix *)malloc(sizeof(struct matrix));
                r1=r1->down;
                r1->down=NULL;
                r1->right=(struct matrix *)malloc(sizeof(struct matrix));
                r1->right=t1;
                r1->down=NULL;

                //linking the non-zero element to it's corresponding column
                (m1[c])->down=t1;
                m1[c]=(m1[c])->down;
                (m1[c])->down=NULL;
            }
            pr=r;
            pa=a;
        }

        //For 2nd matrix
        if(a==2){
            if(pa!=a){
                pr=0;
                d=0;
                pa=0;
            }

            //same procedure as done in the 1st matrix is also followed for the second matrix

            t2->right=(struct matrix *)malloc(sizeof(struct matrix));
            t2=t2->right;
            t2->ri=r;
            t2->ci=c;
            t2->value=v;
            t2->down=NULL;
            t2->right=NULL;

            if(pr==0 || pr==r){
                if(d==0){
                    d=1;
                    if(pr==0){
                        r2->value=r;
                        r2->right=(struct matrix *)malloc(sizeof(struct matrix));
                        r2->right=t2;
                        r2->down=NULL;
                    }
                }
                (m2[c])->down=t2;
                m2[c]=(m2[c])->down;
                (m2[c])->down=NULL;
            }
            else{
                d=0;
                r2->down=(struct matrix *)malloc(sizeof(struct matrix));
                r2=r2->down;
                r2->down=NULL;
                r2->right=(struct matrix *)malloc(sizeof(struct matrix));
                r2->right=t2;
                r2->down=NULL;

                (m2[c])->down=t2;
                m2[c]=(m2[c])->down;
                (m2[c])->down=NULL;
            }
            pr=r;
            pa=a;
        }
    }
    //For optimizing the code :- Removing the NULL column header Node(Columns in which no value is there)
    //To make data structure of O(m) space
    
    p1=col_head1;
    p2=col_head2;

    for(i=0;i<=n;i++){
        if(p1==NULL){   
            break;
        }
        if(p1->right!=NULL){
            if(p1->right->down==NULL){  //breaking the link of the empty column header node and then connecting
                temp=p1->right;         //the previous node to the next node for the 1st matrix
                p1=p1->right->right;    
                free(temp);             // freeing space of the empty column header
            }
        }
    }

    for(i=0;i<=n;i++){
        if(p2==NULL){
            break;
        }

        if(p2->right!=NULL){
            if(p2->right->down==NULL){   //breaking the link of the empty column header node and then connecting
                temp=p2->right;          //the previous node to the next node for the 2nd matrix
                p2=p2->right->right;
                free(temp);              // freeing space of the empty column header
            }
        }
    }
}

void multiply(struct matrix *col_head2,struct matrix *row_head1,struct matrix *head3 ){

    //Checking if any matrix is NULL matrix or not
    if(row_head1->right==NULL || col_head2->right==NULL){
        return;
    }
    
    //declaring and initialising all the required temporary variables and pointers to create solution matrix
    //of product of two matrices

    unsigned int r;
    struct matrix *t1,*t3,*p2,*k,*l;

    t1=row_head1->right;
    t3=head3;
    p2=col_head2->right->down;
    k=col_head2->right;
    l=row_head1;

    t3->right=NULL;
    t3->value=0;
    r=0;

    while(1){
        while(1){
            while(1){
                if(t1==NULL){ 
                    break;
                }
                if(p2==NULL || (t1->ri!=r && r!=0)){
                    break;
                }
                else if((t1->ci) == (p2->ri)){
                    //storing multiplied value and iteration both along row in 1st matrix and down the column in 2nd matrix
                    t3->value=(t3->value)+(t1->value)*(p2->value);
                    t3->ri=t1->ri;
                    t3->ci=p2->ci;
                    r=t1->ri;
                    t1=t1->right;
                    p2=p2->down;
                }
                else if((t1->ci) < (p2->ri)){
                    //iterating along row of 1st matrix
                    r=t1->ri;
                    t1=t1->right;
                }
                else{
                    //iterating down the column of second
                    p2=p2->down;
                }
            }
            if(t3->value!=0){
                //iterating to get next value to stored
                t3->right=(struct matrix *)malloc(sizeof(struct matrix));
                t3=t3->right;
                t3->right=NULL;
                t3->down=NULL;
                t3->value=0;
            }
            r=0;
            //To start of current row of 1st matrix
            t1=l->right;
            //iterating to next column of 2nd matrix
            k=k->right;
            if(k==NULL){
                break;
            }
            p2=k->down;
        }
        //iterating to next row of 1st matrix
        l=l->down;
        if(l==NULL){
            break;
        }
        t1=l->right;
        //To start of 2nd matrix
        k=col_head2->right;
        if(k==NULL){
            break;
        }
        p2=col_head2->right->down;
    }
    return;
}


int Ifib(long long int n){
    int a=1,b=0,temp;
    if(n==0 || n==1){
        a=n;
    }
    else{
        while(n>=2){
            temp=a;
            a=(a+b)%2021;
            b=temp;
            n--;
        }
    }
    return a;
}

// void* thread_function1(void *arg)
// {
//     int i = 0,val;
//     long long int n = 35;
//     while(i < 120){
//         i++;
//         val = Rfib(n);
//     }
//     printf("done thread1 \n");
// }

void* thread_function2(void *arg)
{
	int i = 0,val;
    long long int n = 38;
    while(i < 25){
        i++;
        val = Rfib(n);
    }
    printf("done thread2 \n");
}
// void* thread_function3(void *arg)
// {
//     unsigned int n = 100000;     //n=size of matrix
// 	int i = 0;
//     while(i < 14){
//         i++;
//         struct matrix *col_head1,*col_head2,*row_head1,*row_head2,*head1,*head2,*head3;

//         //allocating dynamic space for each pointer and initialising them to NULL

//         col_head1=(struct matrix *)malloc(sizeof(struct matrix));
//         col_head2=(struct matrix *)malloc(sizeof(struct matrix));
//         row_head1=(struct matrix *)malloc(sizeof(struct matrix));
//         row_head2=(struct matrix *)malloc(sizeof(struct matrix));
        
//         col_head1->right=NULL;
//         col_head2->right=NULL;
//         row_head1->right=NULL;
//         row_head2->right=NULL;
        
//         col_head1->down=NULL;
//         col_head2->down=NULL;
//         row_head1->down=NULL;
//         row_head2->down=NULL;

//         col_head1->value=0;
//         col_head2->value=0;
//         row_head1->value=0;
//         row_head2->value=0;

//         head1=(struct matrix *)malloc(sizeof(struct matrix));
//         head2=(struct matrix *)malloc(sizeof(struct matrix));
//         head3=(struct matrix *)malloc(sizeof(struct matrix));

//         head1->right=NULL;head1->down=NULL;head1->value=0;
//         head2->right=NULL;head2->down=NULL;head2->value=0;
//         head3->right=NULL;head3->down=NULL;head3->value=0;
        
//         //function create reads the two matrices and links the non zero elements between them so that the matrix
//         //operations can be done efficiently

//         create(n,col_head1,col_head2,row_head1,row_head2,head1,head2);

//         //function multiply takes the input of two matrices and then multiplies them to form the soluton matrix

//         multiply(col_head2,row_head1,head3);
        
//     }
//     printf("done thread3 \n");
// }


void* thread_function4(void *arg)
{
    unsigned int n = 100000000,val;
	int i = 0;
    while(i < 25){
        val = Ifib(n);
        i++;
    }
    printf("done thread4 \n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0)
    {
        printf("Number of threads must be a positive integer\n");
        return 1;
    }

    int n = 2;

    pthread_t tid;
    pthread_t tids[num_threads * n];

    printf("Hello\n");
    for (int i = 0; i < num_threads; i++)
    {
        // pthread_create(&tid, NULL, &thread_function1, NULL);
        // tids[i * 4] = tid;

        pthread_create(&tid, NULL, &thread_function2, NULL);
        tids[i * n + 0] = tid;

        // pthread_create(&tid, NULL, &thread_function3, NULL);
        // tids[i * 4 + 2] = tid;

        pthread_create(&tid, NULL, &thread_function4, NULL);
        tids[i * n + 1] = tid;
    }

    syscall(548);

//    for(int i=0;i<num_threads;i++) pthread_join(tids[i], NULL);

    printf("Bye\n");
    return 0;
}
