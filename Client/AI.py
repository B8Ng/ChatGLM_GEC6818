from zhipuai import ZhipuAI
client = ZhipuAI(api_key="629cb4028132ea7e4ad52dc2ed4f9834.NMNWSfRNwkOuggmJ")
import sys
import re
def remove_emoticons(text):
    # 定义正则表达式匹配颜文字
    emoticon_pattern = re.compile(
        r'[\U0001F600-\U0001F64F]|'  # Emoticons
        r'[\U0001F300-\U0001F5FF]|'  # Symbols & Pictographs
        r'[\U0001F680-\U0001F6FF]|'  # Transport & Map Symbols
        r'[\U0001F700-\U0001F77F]|'  # Alchemical Symbols
        r'[\U0001F780-\U0001F7FF]|'  # Geometric Shapes Extended
        r'[\U0001F800-\U0001F8FF]|'  # Supplemental Arrows-C
        r'[\U0001F900-\U0001F9FF]|'  # Supplemental Symbols and Pictographs
        r'[\U0001FA00-\U0001FA6F]|'  # Chess Symbols
        r'[\U0001FA70-\U0001FAFF]|'  # Symbols and Pictographs Extended-A
        r'[\U00002702-\U000027B0]|'  # Dingbats
        r'[\U0001F680-\U0001F6FF]|'  # Transport and Map Symbols
        r'[\U0001F1E0-\U0001F1FF]'   # Flags (iOS)
        , flags=re.UNICODE)
    
    # 用空字符串替换匹配到的颜文字
    cleaned_text = emoticon_pattern.sub(r'', text)
    return cleaned_text


if len(sys.argv)!=2:
    print("输入为空,会话退出")
    sys.exit()

Problem = sys.argv[1]

# pip install zhipuai 请先在终端进行安装

response = client.chat.completions.create(
model="glm-4-flash",
    messages=[
        {
            "role": "system",
            "content": "你是一个乐于解答各种问题的助手，你的任务是为用户提供专业、准确、有见地的建议。" 
        },
        {
            "role": "user",
            "content": Problem
        }
    ],
    top_p= 0.7,
    temperature= 0.95,
    max_tokens=400,
    tools = [{"type":"web_search","web_search":{"search_result":True}}],
)

buf = response.choices[0].message
buf = str(buf)#转化成str进行处理
buf = buf.replace("\\n","")#除去换行
buf = remove_emoticons(buf)#除去表情
print(buf[27:-37])






