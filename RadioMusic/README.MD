
Radio Music New Firmware

Extra files are no longer needed but have to patch SD/File.cpp.

In the available function change

```
return n > 0X7FFF ? 0X7FFF : n;
```

to

```
return n;
```

 