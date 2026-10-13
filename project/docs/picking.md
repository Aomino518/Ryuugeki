# SceneWindow のクリック選択

Debug / Development の SceneWindow 内でモデルを左クリックすると、Hierarchy と Inspector の選択が切り替わります。何も当たらない場所をクリックすると選択を解除します。画像外の余白、ほかの UI 上、右ボタン操作中、Alt 押下中は選択を変更しません。

## 対象の登録

既存の `Editor::RegisterModel("名前", entity)` に登録済みで、そのフレームに `Entity3D::Draw()` されたモデルが対象です。未登録の敵・弾・地形などは対象になりません。登録したオブジェクトの寿命は既存の Editor と同じ管理です。登録ポインターを残したままモデルを破棄せず、シーン終了時は `Editor::Clear()` で登録をクリアしてください。

背景などを対象から外す場合は `entity->SetPickable(false)` を指定します。スカイドームには設定済みです。Hierarchy からの選択・編集は引き続き利用できます。

## 実装

- Windows SDK の `DirectXCollision.h` を使用します。追加のパッケージやリンクライブラリは不要です。
- Model 読み込み時にローカル座標の BoundingBox を一度作成します。
- SceneWindow の実画像の矩形からクリック位置を正規化します。
- 描画に使った WVP を逆変換して、near ～ far のローカルレイを生成します。ルートノード変換、通常／デバッグカメラ、回転、非均一・負のスケールを同じ行列で扱います。
- BoundingBox の交差後、DirectXCollision の TriangleTests でインデックス付き三角形を判定します。
- ヒット位置を投影した深度で比較し、一番手前の登録モデルを選択します。

三角形の両面を判定します。テクスチャの透明部分、マテリアルの透明度、未登録オブジェクトによる遮蔽は判定しません。輪郭のハイライトや移動ギズモは含みません。

## 確認

`tests/PickingTests.cpp` は GPU 不要の交差判定テストです。Visual Studio の x64 Native Tools Command Prompt で、プロジェクト直下から実行できます。

```bat
mkdir build-check\picking
cl /nologo /std:c++20 /EHsc /W4 /WX /I SeekerEngine\3D /I SeekerEngine\Math tests\PickingTests.cpp /Fo:build-check\picking\PickingTests.obj /Fe:build-check\picking\PickingTests.exe
build-check\picking\PickingTests.exe
```

画面での確認項目:

1. タイトル画面の player をクリックし、Inspector の Name が player になる。
2. 空や SceneWindow の画像外をクリックし、空では解除、画像外では維持される。
3. SceneWindow のサイズやドッキング位置を変えても同じ場所を選択できる。
4. デバッグカメラへ切り替えても選択でき、右ドラッグでは選択が変わらない。
5. シーン切り替え後に古いモデルが選択対象に残らない。
