# EduBfM

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

## Report

Write into [REPORT.md](REPORT.md)
