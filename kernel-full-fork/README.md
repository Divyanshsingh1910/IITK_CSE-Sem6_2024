# Kernel Full Fork

## Members

- Divyansh
- Divyansh Chhabria
- Rajeev Kumar
- Sandeep Nitharwal
- Soham Amit Bharambe


## Mentor

- Debadutta Mishra

***
#### Instructions 
- We have written snippets in code/snippets of our repository which contains the changes we did to kernel code.
- Also the entire source code of the modified kernel can be found here: [link](https://iitk-my.sharepoint.com/:f:/g/personal/divyansh21_iitk_ac_in/EoV00fHmAjhLnFMiYgOZ3ykBuKMRj0vhwlAp-JpLomeR4Q?e=YEhPaN)

#### Running Instructions 
- CRIU 
    We need to do `make install` inside the directory but there are many system related depedencies we needs to be installed accordinly,
    Here are some of them 
    ```
        sudo apt-get install libbsd-dev
        sudo apt-get install libnftables-dev
        sudo apt-get install asciidoc
        sudo apt-get install libprotobuf-dev
    ```
    ```
       	setcap cap_checkpoint_restore+eip /usr/bin/criu
    ```

- FULLFORK & PTREE_CLONE
    For this, there are modules which just needs to be inserted, makefiles are provided, and can be used.
    Note: Kernel source code modification should be done beforehand.
