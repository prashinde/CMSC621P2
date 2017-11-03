#!/bin/bash
echo  $(ps ax|grep node | cut -d' ' -f1)
kill -9 $(ps ax|grep node | cut -d' ' -f1)
