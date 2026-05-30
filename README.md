# EchoHubProject
This is university project, where we aim to write our own version of a desktop voice/text chat application.  
(*In case that this app name is already taken, just let us know and we will change this project's name.*)

# Cheat sheet 
## Cloning
Cloning with submodules:  
```shell
git clone --recurse-submodules https://github.com/Morswin/EchoHubProject
```  
Activating submodules if forgotten to do that when cloning:  
```shell
git submodule update --init --recursive
```  

## Compiling
If you're trying to compile this on windows, try running this batch file, or read it's [contents](https://github.com/Morswin/EchoHubProject/blob/main/compile.bat) to see how we suggest to use cmake to help yourself compile this repo.
```shell
compile.bat
```

### Prerequisites
You need to have intsalled `cmake` to use this script. It also assumes that you're using windows. You can get the compiler with the Visual Studio Installer.  
For tips how to compile this on macos or linux please rise an issue in this project.
