#!/bin/bash
echo  $(ps ax|grep node | cut -d' ' -f2)
kill -9 $(ps ax|grep node | cut -d' ' -f2)
