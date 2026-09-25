-- Implicit invariant: render_figs.py renders each fig-*.svg to a sibling
-- fig-*.pdf. pdflatex/lualatex cannot embed SVG, so this filter points pandoc
-- at the PDF twin without touching the Markdown.

function Image(el)
  if el.src and el.src:match("%.svg$") then
    el.src = el.src:gsub("%.svg$", ".pdf")
    if el.attributes.width == nil then
      el.attributes.width = "100%"
    end
  end
  return el
end
