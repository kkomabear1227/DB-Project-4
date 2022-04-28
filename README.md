# EduBtM

This is a project 4 of CSED421

## Authentification setting

In auth.yaml

```
student_id: <your student id>
password: <your registration password>
```


To check correctness

```
cd auth_test
bash auth_test.sh
```

## Testing

![Test](test_structure.png)

Autograder runs 1) correctness test and 2) performance test.

Correctness Test

- Outputs error
- Workloads : 15

Performance Test

- Outputs runtime
- Workloads : 15

```
cd test
bash autograding.sh
```

### How To Dump a Page

To show detailed description about page, please use dump page API like below.


```
printf("****************************** Dump  ******************************\n");
printf("pageNo of rootPid : %d\n", rootPid.pageNo);
dumpPage.volNo = volId;
printf("Enter the pageNo : ");
if (scanf("%d", &(dumpPage.pageNo)) == 0)
{
  while(getchar() != '\n');
  printf("Wrong number!!!\n");
  break;
}

e = dumpBtreePage(&dumpPage, kdesc);
if (e == eBADBTREEPAGE_BTM) printf("The page (PID: ( %d, %d )) does not exist in the B+ tree index.\n", dumpPage.volNo, dumpPage. pageNo);
        else if (e < eNOERROR) ERR(e);
```

## Report

Write into [REPORT.md](REPORT.md)
