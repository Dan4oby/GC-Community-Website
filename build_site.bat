chcp 65001 >nul

pandoc "index.md" -f markdown+wikilinks_title_after_pipe -s -o html/index.html --self-contained --css=style.css --resource-path=images

pandoc "fof.md" -f markdown+wikilinks_title_after_pipe -s -o html/fof.html --self-contained --css=style.css --resource-path=images

pandoc "gclauncher.md" -f markdown+wikilinks_title_after_pipe -s -o html/gclauncher.html --self-contained --css=style.css --resource-path=images

