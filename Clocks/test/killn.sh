#!/bin/bash
kill -9 $(ps ax|grep node | cut -d' ' -f2)
