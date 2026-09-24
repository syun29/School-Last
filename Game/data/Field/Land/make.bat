@echo off
rem objファイルをfieldファイルへ変換
rem -f 対象ファイル -l メッシュ分割距離　-m メッシュ分割数　-c 衝突判定用メッシュ分割
@echo on

ConvertField.exe -f land.obj -m 12 -c 8