# Build cyclic_quorum.pdf and cyclic_quorum-talk.pdf. Run from paper/.
#
# Flag rationale (md-to-latex / md-to-beamer skills, so the knowledge is not
# tribal):
#  - pandoc 3.11 ignores positional *.yaml, so metadata goes through
#    --metadata-file.
#  - The paper headings start at "##", so --shift-heading-level-by=-1 maps them
#    to \section (otherwise numbering degrades to 0.1, 0.2).
#  - documentclass: IEEEtran (set in the front matter) gives the two-column
#    layout; the wide tables are raw LaTeX table* floats so they span both
#    columns (pandoc's longtable cannot be used in two-column mode).
#  - pdflatex/lualatex cannot embed SVG; `make figs` renders each fig-*.svg to
#    a PDF twin and svg2pdf.lua points pandoc at it.
#  - The beamer talk loads its preamble from the CLI, because pandoc
#    Markdown-mangles a YAML header-includes.

PANDOC := pandoc
FLAGS := -f markdown+tex_math_single_backslash \
         --citeproc -s -t latex -N --reference-links \
         --pdf-engine=pdflatex \
         --shift-heading-level-by=-1 \
         --lua-filter=svg2pdf.lua \
         --metadata-file=latex.yaml

TALKFLAGS := -t beamer --pdf-engine=lualatex \
             --lua-filter=svg2pdf.lua \
             --include-in-header=preamble.tex \
             --metadata-file=cyclic_quorum-talk.yaml

.PHONY: paper talk figs clean

figs:
	python render_figs.py

paper: figs
	$(PANDOC) $(FLAGS) cyclic_quorum.md -o cyclic_quorum.pdf

talk: figs
	$(PANDOC) $(TALKFLAGS) cyclic_quorum-talk.md -o cyclic_quorum-talk.pdf

clean:
	$(RM) cyclic_quorum.pdf cyclic_quorum-talk.pdf $(wildcard fig-*.pdf)
