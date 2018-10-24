2018/10/24 by Shingo Nakamrua

■ 概要
- つくばチャレンジ用プログラム．
- JetsonStatus を 共有メモリへ読書きするためのソースファイル．


■ ソースファイル
  src/shmJetsonStatus.cpp  メインソースファイル
  src/shmJetsonStatus.h    ヘッダファイル
  src/test-writer.cpp      書込サンプルプログラム
  src/test-reader.cpp      読込サンプルプログラム


■ 使い方

 A.書き込み
  書き込みには，名前空間 sn の JetsonStatusWriter クラスを使用する．
  このクラスの静的メソッドを使用して共有メモリにJetsonStatusを書き込む．
  使い方の流れは以下の通り．
  
    1. bool JetsonStatusWriter::Open(void) でオープン
    2. bool JetsonStatusWriter::Write( JetasonStatus s ) で書き込む
    3. bool JetsonStatusWriter::Close(void) でクローズ
  
  1 と 3 はプログラムの最初と最後に行えばよい．
  2 はオープン中であればループ処理などで何度も書込可能．
  どのメソッドも成功すれば true が，失敗すれば false が返る．
  詳しくは test-writer.cpp を参考にする．


 B. 読み込み
  読み込みは、名前空間 sn の getJetsonStatus 関数を使うだけ．
  仕様は以下の通り．

    bool getJetsonStatus( JetsonSatus *status );

  引数にデータを取得するための JetsonStatus のアドレスを渡す．
  成功すれば trueが、失敗で false が返る．
  書き込み側の JetsonWriter でオープンしていなければ失敗する．
  詳しくは test-reader.cpp を参考にする．


■ コンパイル時の注意点
  JetsonStatus構造体が必要なので，shmJetsonStatus.h でヘッダーファイル
  "Jetson-COM-DATA.hpp" をインクルードしています. 見つからないようなら
  適切なパスでインクルードすること.


■ サンプルプログラム
  CMake, make でコンパイルすると，binディレクトリが作成され，
  その中に２つの実行ファイル test-writer と test-reader が生成される． 
  先に test-writer を起動してから， test-reader を実行する．

