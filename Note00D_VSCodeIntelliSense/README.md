# Better VSCode IntelliSense for embedded projects

I have found that for my embedded projects VSCode IntelliSense works better
if it is configured to use the ```compile_commands.json```.
This approach has the benefits of sparing messing up with ```includePaths```
in project settings and also other editors use ```compile_commands.json```
for their ```IntelliSense``` version.

To generate ```compile_commands.json``` from the project's Makefile I use
[compiledb](https://github.com/nickdiego/compiledb) with the command:

```
compiledb -n make
```

Then following [this article](https://code.visualstudio.com/docs/cpp/faq-cpp)
I set path to the generated file with:
**C/C++: Edit Configurations (UI) > Advanced > Compile commands**
