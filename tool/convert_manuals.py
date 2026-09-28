#!/usr/bin/env python3
"""Build page-addressable Markdown reference copies of the pinned PDF manuals.

Requires macOS PDFKit, Swift and Python 3. No network or third-party packages.
Run from any directory: python3 tool/convert_manuals.py
"""
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'doc' / '_generated' / 'manual'
SOURCE_MANIFEST = ROOT / 'doc' / 'reference' / 'source-manifest.json'
PAGE_RANGE = 50
SLUGS = {
    'R01UH0887EJ0110': 'ra4m1',
    'ESP32-P4 TRM': 'esp32-p4',
    'PM0253': 'pm0253',
    'RM0433': 'rm0433',
    'DS12117': 'stm32h753xi',
    'ES0392': 'es0392',
}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding='utf-8')

def escape(text):
    return re.sub(r'([\\`*_{}\[\]<>|])', r'\\\1', ' '.join(text.split()))

def filename(page, total):
    start = (page - 1) // PAGE_RANGE * PAGE_RANGE + 1
    return f'pages-{start:04d}-{min(start + PAGE_RANGE - 1, total):04d}.md'

def page_link(page, total):
    return f'{filename(page, total)}#pdf-page-{page}'

def convert(document, extracted):
    pages = extracted['pages']
    total = document['pages']
    assert len(pages) == total, document['path']
    assert [p['number'] for p in pages] == list(range(1, total + 1))
    bookmarks = extracted['bookmarks']
    folder = OUTPUT / SLUGS[document['document_id']]
    pdf_link = '../../../_vendor/manual/' + Path(document['path']).name
    title = document['title']
    empty = [p['number'] for p in pages if not p['text'].strip()]
    expected = {'README.md'}
    for start in range(1, total + 1, PAGE_RANGE):
        end = min(start + PAGE_RANGE - 1, total)
        name = filename(start, total)
        expected.add(name)
        body = [f'# {title}: PDF pages {start}–{end}', '',
                f'[Document index](README.md) · [Original PDF]({pdf_link}#page={start})', '',
                '> Search/reference extraction. Text follows PDFKit reading order. Tables, equations and diagrams require checking the linked PDF.', '']
        for page in pages[start-1:end]:
            n = page['number']
            body += [f'## PDF page {n}', '',
                     f'[View original page]({pdf_link}#page={n})', '']
            text = page['text'].replace('\r\n', '\n').replace('\r', '\n')
            if text.strip():
                # A fence protects register symbols, underscores and table-like text
                # from accidental Markdown interpretation. No words are summarized.
                fence = '`' * max(3, 1 + max((len(m[0]) for m in re.finditer(r'`+', text)), default=0))
                body += [fence + 'text', text, fence, '']
            else:
                body += ['> No text layer was extracted on this page. Inspect the original PDF; this page may be blank or contain only graphics.', '']
        write(folder / name, '\n'.join(body))
    index = [f'# {title}', '',
             f"Document: **{document['document_id']}**, revision **{document['revision']}**, {total:,} PDF pages.", '',
             f'[All manuals](../README.md) · [Original PDF]({pdf_link}) · [Pinned sources](../../../reference/source-manifest.json)', '',
             f"Source SHA-256: `{document['sha256']}`", '',
             'This is a searchable extraction of the embedded text, not a semantic reconstruction of the PDF. Page numbers below are physical PDF pages (1-based), which can differ from printed page labels. Original text is fenced to preserve register notation and avoid interpreting it as Markdown. Headers, footers and hyphenation are retained. OCR was not used. Figures are accessed through the original-page links.', '',
             '## Page ranges', '']
    index += [f'- [PDF pages {start}–{min(start+PAGE_RANGE-1,total)}]({filename(start,total)})' for start in range(1,total+1,PAGE_RANGE)]
    index += ['', '## PDF bookmarks', '', 'Navigation below comes from the PDF outline. Missing or inaccurate PDF bookmarks are not inferred or corrected.', '']
    if bookmarks:
        index += [f"{'  ' * b['depth']}- [{escape(b['title'])}]({page_link(b['page'],total)}) — PDF page {b['page']}" for b in bookmarks]
    else:
        index += ['This PDF has no usable destination bookmarks; use the page ranges or text search.']
    index += ['', '## Extraction checks', '',
              f'- Represented pages: {len(pages):,} / {total:,}.',
              f'- Pages with extracted text: {total-len(empty):,}.',
              f'- Source bookmarks: {len(bookmarks):,}.',
              '- Page text is preserved without summarization or attempted table-column repair.',
              '- Register values, bit positions, reserved fields, tables and diagrams must be verified against the PDF before implementing hardware descriptions.', '']
    if empty:
        index += ['Pages without extracted text: ' + ', '.join(f'[{n}]({page_link(n,total)})' for n in empty) + '.', '']
    write(folder / 'README.md', '\n'.join(index))
    # Never delete user-authored files. Refuse stale generated ranges instead.
    stale = {p.name for p in folder.glob('pages-*.md')} - expected
    if stale:
        raise RuntimeError(f'Stale page ranges need review in {folder}: {sorted(stale)}')
    outputs = []
    for name in sorted(expected):
        path = folder / name
        outputs.append({'path': str(path.relative_to(ROOT)), 'sha256': digest(path.read_bytes())})
    return {'document_id': document['document_id'], 'source_path': document['path'],
            'source_sha256': document['sha256'], 'index': str((folder/'README.md').relative_to(ROOT)),
            'pages': total, 'pages_with_text': total-len(empty), 'empty_text_pages': empty,
            'bookmark_count': len(bookmarks), 'outputs': outputs}

def main():
    manifest = json.loads(SOURCE_MANIFEST.read_text())
    documents = manifest['documents']
    # Fail before writing anything if a source changed without an explicit re-pin.
    for document in documents:
        data = (ROOT/document['path']).read_bytes()
        if digest(data) != document['sha256']:
            raise RuntimeError(f"Source changed: {document['path']}; review and update the source manifest first")
    with tempfile.TemporaryDirectory(prefix='mcuhw-markdown-') as temporary:
        temp = Path(temporary)
        env = dict(os.environ, CLANG_MODULE_CACHE_PATH=str(temp/'module-cache'))
        binary = temp/'extract-pdf'
        subprocess.run(['swiftc',str(ROOT/'tool/extract_pdf.swift'),'-o',str(binary)],check=True,env=env)
        def extract(document):
            path = temp/(SLUGS[document['document_id']] + '.json')
            subprocess.run([str(binary),str(ROOT/document['path']),str(path)],check=True)
            result = json.loads(path.read_text())
            converted = convert(document,result)
            print(f"{document['document_id']}: {converted['pages']:,} pages, {converted['bookmark_count']:,} bookmarks",flush=True)
            return converted
        completed = {}
        with ThreadPoolExecutor(max_workers=2) as pool:
            futures = {pool.submit(extract,d):d['document_id'] for d in documents}
            for future in as_completed(futures):
                completed[futures[future]] = future.result()
    entries = [completed[d['document_id']] for d in documents]
    index = ['# Searchable MCU manuals', '',
             'Generated Markdown reference copies of the six pinned PDFs. Start here to find sections, then follow an original-page link to check authoritative register tables, diagrams or ambiguous text.', '',
             '| Manual | Revision | Pages | Markdown |', '| --- | --- | ---: | --- |']
    for d,e in zip(documents,entries):
        link = str(Path(e['index']).relative_to('doc/_generated/manual'))
        index.append(f"| {d['document_id']} | {d['revision']} | {d['pages']:,} | [{d['title']}]({link}) |")
    index += ['', '## Search and regenerate', '',
              'From the project root:', '', '```sh',
              "rg -n -i 'module.stop|GTIOCA|GPIO matrix|DMA request' doc/_generated/manual --glob 'pages-*.md'",
              'python3 tool/convert_manuals.py', '```', '',
              'The converter requires macOS PDFKit, Swift and Python 3. It uses no network or third-party packages. It checks the pinned PDF checksums before conversion. Each range contains at most 50 physical PDF pages, each with a stable page heading and original-PDF link. PDF bookmarks provide section navigation.', '',
              '## Fidelity and verification', '',
              'Embedded text is extracted with PDFKit and retained inside text fences; this prevents accidental Markdown formatting of register names or tables. This is not a reconstruction of semantic tables or equations, and graphic content is not rendered into the Markdown. Reading order and column alignment can be imperfect. Do not derive bit positions, field values, reset values or pin mappings from extracted text alone: verify the source PDF.', '',
              'Every source page is represented, including pages with no extracted text, which are flagged and linked. No OCR or content summarization is performed. Generated hashes and counts are recorded in [conversion-manifest.json](conversion-manifest.json); authoritative PDF identities remain in [source-manifest.json](../../reference/source-manifest.json).', '']
    write(OUTPUT/'README.md','\n'.join(index))
    conversion = {'schema_version':1,'path_base':'project_root','extractor':'macOS PDFKit page.string and PDF outline',
                  'page_range_size':PAGE_RANGE,'source_manifest_sha256':digest(SOURCE_MANIFEST.read_bytes()),
                  'scripts':{str(p.relative_to(ROOT)):digest(p.read_bytes()) for p in [Path(__file__).resolve(),ROOT/'tool/extract_pdf.swift']},
                  'index_sha256':digest((OUTPUT/'README.md').read_bytes()),'documents':entries}
    write(OUTPUT/'conversion-manifest.json',json.dumps(conversion,indent=2)+'\n')
    print(f"Finished: {sum(e['pages'] for e in entries):,} pages across {len(entries)} manuals.",flush=True)

if __name__ == '__main__':
    main()
