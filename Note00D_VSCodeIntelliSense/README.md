# VSCode IntelliSense for embedded project

I found that for my embedded projects VSCode IntelliSense works better if it is
configured to use ```compile_commands.json```.

To generate ```compile_commands.json``` from the project Makefile I use
[compiledb](https://github.com/nickdiego/compiledb) with the command:

```
compiledb -n make
```

Then following [this article](https://code.visualstudio.com/docs/cpp/faq-cpp)
I set path to the generated file with:
**C/C++: Edit Configurations (UI) > Advanced > Compile commands**
