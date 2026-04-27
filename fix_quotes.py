import re, io
path = r'C:\Users\da983\Desktop\GraphVisualizer\build_thesis.js'
s = io.open(path, encoding='utf-8').read()

# Match p("...") possibly multi-line. Inner content has no escapes (we authored it).
PAT = re.compile(r'p\("([^"]*)"\)', re.DOTALL)

def fix(m):
    inside = m.group(1)
    out = []
    open_ = True
    for ch in inside:
        if ch == '"':
            out.append('\u201C' if open_ else '\u201D')
            open_ = not open_
        else:
            out.append(ch)
    return 'p("' + ''.join(out) + '")'

# Inner content shouldn't contain " at all (we've already structured it). The simpler PAT
# will only match p(...) calls whose inner text has no straight quotes - so it won't help.
# Actually since the issue is exactly the inner ", we need a different pattern.

# Approach: find 'p("' then match until ')' that's preceded only by '"'.
# Easier: process line-by-line, finding `p("` start, scanning forward to the closing `")`.

text = s
result_parts = []
i = 0
n = len(text)
while i < n:
    # Find next p("
    j = text.find('p("', i)
    if j < 0:
        result_parts.append(text[i:])
        break
    result_parts.append(text[i:j])
    # find closing ")  -- the very last " before )
    # The pattern: walk forward and find a `")` sequence (assume no escapes inside)
    k = text.find('")', j+3)
    if k < 0:
        result_parts.append(text[j:])
        break
    inner = text[j+3:k]
    # Convert straight quotes inside
    out = []
    open_ = True
    for ch in inner:
        if ch == '"':
            out.append('\u201C' if open_ else '\u201D')
            open_ = not open_
        else:
            out.append(ch)
    result_parts.append('p("' + ''.join(out) + '")')
    i = k + 2

s2 = ''.join(result_parts)
print('changed:', s != s2)
io.open(path, 'w', encoding='utf-8').write(s2)
