chcp 65001 >nul

pandoc "index.md" -f markdown+wikilinks_title_after_pipe -s -o index.html --self-contained --css=style.css --resource-path=images

