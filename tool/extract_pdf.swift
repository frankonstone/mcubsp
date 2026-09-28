import Foundation
import PDFKit

// Read PDF text and navigation without altering the authoritative source PDF.
// Invoked by convert_manuals.py; output is temporary UTF-8 JSON.
func fail(_ message: String) -> Never {
    FileHandle.standardError.write(Data((message + "\n").utf8))
    exit(1)
}
guard CommandLine.arguments.count == 3 else {
    fail("usage: extract_pdf INPUT.pdf OUTPUT.json")
}
let input = CommandLine.arguments[1]
guard let document = PDFDocument(url: URL(fileURLWithPath: input)), !document.isLocked else {
    fail("Cannot open unlocked PDF: \(input)")
}
var pages: [[String: Any]] = []
for index in 0..<document.pageCount {
    guard let page = document.page(at: index) else { fail("Missing page \(index + 1)") }
    pages.append([
        "number": index + 1,
        "label": page.label ?? String(index + 1),
        "text": page.string ?? ""
    ])
}
var bookmarks: [[String: Any]] = []
func visit(_ outline: PDFOutline, depth: Int) {
    if let label = outline.label, !label.isEmpty {
        let destination = outline.destination ?? (outline.action as? PDFActionGoTo)?.destination
        if let page = destination?.page {
            let index = document.index(for: page)
            if index != NSNotFound && index >= 0 && index < document.pageCount {
                bookmarks.append(["title": label, "depth": depth, "page": index + 1])
            }
        }
    }
    for index in 0..<outline.numberOfChildren {
        if let child = outline.child(at: index) { visit(child, depth: depth + 1) }
    }
}
if let root = document.outlineRoot {
    for index in 0..<root.numberOfChildren {
        if let child = root.child(at: index) { visit(child, depth: 0) }
    }
}
let result: [String: Any] = ["pages": pages, "bookmarks": bookmarks]
let data = try JSONSerialization.data(withJSONObject: result, options: [.sortedKeys])
try data.write(to: URL(fileURLWithPath: CommandLine.arguments[2]), options: .atomic)
