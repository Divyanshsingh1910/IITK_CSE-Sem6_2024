cmd_/home/divyansh/Desktop/CS_614/hello_mod/Module.symvers :=  sed 's/ko$$/o/'  /home/divyansh/Desktop/CS_614/hello_mod/modules.order | scripts/mod/modpost      -o /home/divyansh/Desktop/CS_614/hello_mod/Module.symvers -e -i Module.symvers -T - 