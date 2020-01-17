[TOC]

# 实验一  Linux内核编译及添加系统调用

<font face="楷体" size=4>17042127     陶逸群</font>

## 设计要求

- 添加一个系统调用，实现输出功能
- 添加一个系统调用，实现对指定进程的 nice 值得修改或读取功能
- 写一个简单的应用程序测试添加的修改指定进程nice值的系统调用
-  若程序中调用了 Linux 的内核函数，要求深入阅读相关函数源码

## 实验步骤

1. 下载Linux源码

   我下载了Linux-5.3.5的源码。

   ![image-20191215195212224](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215195212224.png)

2. 在系统调用表中添加系统调用号![image-20191215195544148](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215195544148.png)我在系统调用表中添加了两个系统调用号，其中 335 号用于在内核中输出我的信息，证明我实现了对内核的修改。336号系统调用用于读取或修改进程的 nice 值。

3. 申明系统调用服务例程原型![image-20191215200032114](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215200032114.png)

4. 实现系统调用服务例程

   ```C
   SYSCALL_DEFINE0(tyqsyscall){
           printk("陶逸群的系统调用\n ");
           return 0;
   }
   SYSCALL_DEFINE5(mysetnice,pid_t,pid,int,flag,int,nicevalue,void __user *,prio,void __user *,nice){
       struct pid * kpid;//进程描述符指针，指向一个枚举类型
       struct task_struct * task;//任务描述符信息
       kpid = find_get_pid(pid);//根据进程号返回kpid
       task = pid_task(kpid, PIDTYPE_PID);//返回task_struct
       int n;
       n = task_nice(task);//返回进程当前nice值 
       int p;              
       p = task_prio(task);//返回进程当前prio值
       if(flag == 1){
           set_user_nice(task, nicevalue);//修改进程nice值 
           n = task_nice(task);//重新取得进程nice值
           p = task_prio(task);//重新取得进程当前prio值
           copy_to_user(nice,&n,sizeof(n));//将nice值拷贝到用户空间
           copy_to_user(prio,&p,sizeof(p));//将prio值拷贝到用户空间
           return 0;
       }
       else if(flag == 0){
           copy_to_user(nice,&n,sizeof(n));//将nice值拷贝到用户空间
           copy_to_user(prio,&p,sizeof(p));//将prio值拷贝到用户空间
           return 0;
       }
       return EFAULT;
   }
   ```

   内核函数的具体作用见以上注释部分。

5. 重新编译内核

6. 编写用户态程序来测试新添加的系统调用

   - 测试打印信息

     ```C
     #include <unistd.h>
     #include <sys/syscall.h>
     int main(){
     	syscall(335);
     	return 0;
     }
     ```

   - 测试修改nice值

     ```C
     #define _GNU_SOURCE
     #include <unistd.h>
     #include<sys/syscall.h>
     #include<stdio.h>
     #include<stdlib.h>
     int main(){
         pid_t pid;
         int nicevalue;
         int flag;
         int p = 0;
         int n = 0;
         int *prio;
         int *nice;
         prio = &p;
         nice = &n;
     
         printf("请输入pid：\n");
         scanf("%d",&pid);
         printf("请输入nice：\n");
         scanf("%d",&nicevalue);
     
         printf("请输入flag：\n");
         scanf("%d",&flag);
     
         syscall(336,pid,flag,nicevalue,prio,nice);
     
         printf("现在的nice为%d\n,prio为%d\n",n,p);
         return 0;
     }
     
     ```

## 结果演示

![image-20191215201746807](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215201746807.png)

修改前：

![image-20191215202007062](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215202007062.png)

修改：

![image-20191215202101166](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215202101166.png)

修改后：![image-20191215202208340](C:\Users\10059\AppData\Roaming\Typora\typora-user-images\image-20191215202208340.png)

## 体会

通过本次实验，我对 Linux 有了初步的了解，能够掌握 Linux 系统的基本使用，同时，通过本次实验我了解了 Linux 内核的修改、添加系统调用以及内核编译方法，并通过自己的实验实现了两个系统调用。 同时通过本次实验我还掌握了一些内核函数比如 set_user_nice()以及 get _task()，同时对进程的相关知识有了更加深入的理解。