<p align="center">
<a href="https://github.com/jpedr0c/42_webserv">
<img src="./webserv.png" height="120" width="120">
</a>
</p>
<h1 align=center>
  <strong> WEBSERV </strong>
</h1>

<p align="center">
  <sub> This project aims to create a minimal C shell capable of executing some basic Unix commands.
  <sub>
</p>


[![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)](#table-of-contents)

<p align="center">
  <a href="#About"> 💡 About the project </a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
  <a href="#Explanation"> 📝 Explanation </a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
  <a href="#HowUse"> ⚙️ How to use</a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
  <a href="#Credits"> 🏆 Credits</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
</p>

<br/>

<a id="About"></a>
## 💡 About the project
> This project is part of 42's curriculum and aims to create a basic shell in C, capable of executing some basic Unix commands. In this project we learned more about dealing with reading and interpreting command line commands, managing processes, and manipulating files and directories. After completing the project we gained a good understanding of the dynamics of Unix shells.

<br/>

<a id="Explanation"></a>
## 📝 Explanation

- Minishell is a C language program that simulates a shell, which is a command-line interface that allows the user to execute commands and programs on the operating system. When running Minishell, the user can enter commands such as `ls`, `cd`, `echo`, `pwd`, `exit`, `cat`, `grep`, redirection (`<`, `>`) and heredocs (`<<`), which will be executed by the program.

- It's built around an infinite loop that reads user inputs line by line. It interprets each line as a command to be executed and parses the input to identify the command name and its arguments.

- Minishell is also responsible for managing processes. When the user enters a command that needs to create a new process, Minishell is responsible for creating that process and executing the program associated with it. Minishell is also capable of handling interruption signals, such as CTRL+C, and redirecting input and output to files.

<br/>

<a id="HowUse"></a>
## ⚙️ How to use

1. Clone this repository
```sh
git clone https://github.com/jpedr0c/42_minishell.git
```
2. In the project directory, compile the program using the following command
```sh
make
```
3. Run Minishell
```sh
./minishell
```
4. If you want to remove the object files generated during compilation
```sh
make clean
```
5. If you want to remove the object files and executable generated during compilation
```sh
make fclean
```

<br/>

<a id="Credits"></a>
## 🏆 Credits
<h3 align="center">Thank you for taking the time to review our project!!!</h3>
    
<h4>The members of our team include:</h4>   
<div>
  
| [<img src="https://avatars.githubusercontent.com/u/78514252?v=4" width="120"><br><p align="center"> João Pedro </p>](https://github.com/jpedr0c)[<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/github-dark.svg" width="25" height="25" />](https://www.github.com/jpedr0c) [<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/linkedin.svg" width="25" height="25" />](https://www.linkedin.com/in/jpedroc) | [<img src="https://avatars.githubusercontent.com/u/57332018?v=4" width="120"><br><p align="center"> Raoni Silva </p>](https://github.com/raonieqr)[<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/github-dark.svg" width="25" height="25" />](https://www.github.com/raonieqr) [<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/linkedin.svg" width="25" height="25" />](https://www.linkedin.com/in/raonieqr/) | [<img src="https://avatars.githubusercontent.com/u/78514252?v=4" width="120"><br><p align="center"> João Pedro </p>](https://github.com/jpedr0c)[<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/github-dark.svg" width="25" height="25" />](https://www.github.com/jpedr0c) [<img src="https://raw.githubusercontent.com/danielcranney/readme-generator/main/public/icons/socials/linkedin.svg" width="25" height="25" />](https://www.linkedin.com/in/jpedroc) |
|---|---|
  
</div>
