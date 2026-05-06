import re
import csv

def parse_mime_types_to_csv(text_content):
    # 正则表达式匹配MIME类型和文件扩展名
    pattern = r'^([a-z][a-z0-9.+-]*/[a-z0-9.+-]+)\s+((?:\.[a-z0-9]+(?:\s+)?)+)'
    
    results = []
    
    for line in text_content.split('\n'):
        line = line.strip()
        if not line:
            continue
            
        # 使用正则表达式匹配
        match = re.match(pattern, line)
        if match:
            mime_type = match.group(1)
            extensions = match.group(2).split()
            
            for ext in extensions:
                results.append([mime_type, ext])
    
    return results

# 使用示例
text_content = """application/acrobat .pdf
application/andrew-inset .ez ATK inset
application/annodex .anx Annodex exchange format
application/appinstaller .appinstaller Windows app store installer"""

# 解析数据
mime_data = parse_mime_types_to_csv(text_content)

# 写入CSV文件
with open('mime_types.csv', 'w', newline='', encoding='utf-8') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['MIME Type', 'File Extension'])  # 表头
    writer.writerows(mime_data)

print(f"成功生成 {len(mime_data)} 行数据到 mime_types.csv")