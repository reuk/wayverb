to add a submodule
------------------

```
git submodule add <address> <optional dir>
```

to use submodules after checking out the project
------------------------------------------------

```
git submodule init
git submodule update
```

OR just clone the project with `--recursive`

```
git clone --recursive <the project>
```

to update a submodule
---------------------

```
cd <the submodule>
git checkout master
git pull
cd <the root dir of the project>
git add <the submodule>
git commit -m "updated <the submodule>"
```
