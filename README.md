# Jam STAPL for Arduino

Jam STAPL の Arduino 実装

https://www.intel.com/content/www/us/en/programmable/support/support-resources/download/programming/jam.html

https://github.com/margro/jam-stapl

  

# リポジトリ構造

 - Document （ドキュメント類）
 - JamSTAPL （Jam STAPL Playerコード）
 - JamSTAPL_Lib （Jam STAPL Playerライブラリ）
 - JamSTAPL_BC （Jam STAPL Byte-Code Playerコード）
 - JamSTAPL_BC_Lib （Jam STAPL Byte-Code Playerライブラリ）
 - Source （Intel配布オリジナルコード）


  

# Jam STAPL Player

テキスト形式のJam STAPLファイル（.jam）用

https://www.intel.com/content/www/us/en/programmable/support/support-resources/download/legacy/jam/dnl-player.html

  



現段階未実装



  

# Jam STAPL Byte-Code Player

バイトコードのJam STAPLファイル（.jbc）用

https://www.intel.com/content/www/us/en/programmable/support/support-resources/download/legacy/jam/dnl-byte_code_player.html

  

## 動作確認済みボード

 - Arduino Due（ボード定義必須）
 - ESP32-DevKitC（ボード定義必須）
 - Raspberry Pi Pico（ボード定義必須）
 - STM32F746G-DISCO（ボード定義・STM32SD/FatFsライブラリ必須）

  

## 実装方針

上記URLからのjbi_22.exeを解凍し得られたソースコードを元に実装。

変更点は、jbiport.h内でEMBEDDEDを宣言していること、jbistub.cを削除。

またSTM32duino環境下でRESET宣言が重複するため、jbijtag.c/h中のdefineを書き換え。

jbimain.hを新規で用意した。

削除したjbistub.cをベースにjbimcu.cpp/hにArduino用に再実装。

スケッチJamSTAPL_BC.inoとのインターフェースはjbiarduino.cpp/h。

  

| ファイル名 | 状況 |
----|----
| JamSTAPL_BC.ino | 新規作成・スケッチメインファイル |
| jbiarduino.cpp | 新規作成・スケッチインターフェース |
| jbiarduino.h | 新規作成・スケッチインターフェース |
| jbicomp.c | 配布コード使用・未編集 |
| jbicomp.h | 配布コード使用・未編集 |
| jbiexprt.h | 配布コード使用・未編集 |
| jbijtag.c | 配布コード使用・RESET宣言書き換え |
| jbijtag.h | 配布コード使用・RESET宣言書き換え |
| jbimain.c | 配布コード使用・未編集 |
| jbimain.h | 新規作成 |
| jbimcu.cpp | 新規作成・jbistub.cベース |
| jbimcu.h | 新規作成・jbistub.cベース |
| jbiport.h | 配布コード使用・EMBEDDED宣言書き換え |

  

## 次の課題

 - jbimcu.cpp/h の整備（jbistub.cを完全に実装しきれていない）
 - 関数「jbi_message」をキャラクタLCD等に渡すコールバック実装？（実行中にStream以外で進捗確認）
 - mallocがうまく動作していない疑惑がある（MKRZERO等メモリが足りない環境でもmallocのエラーが出ない？）
 - ライブラリとのファイル参照渡しを考える（SD.hをインクルードしないとFileクラスが使えない）
 - ESP32等でWifiやSPIFFSからのアップデートを実行できるように
 - スタティックライブラリ化
 - スタティックライブラリのビルドスクリプトを整備
 - 独自ファイル形式対応（JBCファイルに独自ヘッダを付けて管理上バージョンを読み取れるように）




  


