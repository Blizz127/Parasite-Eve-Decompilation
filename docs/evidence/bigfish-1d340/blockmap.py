import re,sys,json
lines=open('asm/disc1/DB40.s').read().splitlines()
start=end=None
for i,l in enumerate(lines):
    if 'glabel func_8001D340' in l: start=i
    if 'endlabel func_8001D340' in l: end=i;break
region=lines[start:end+1]
addr_re=re.compile(r'/\*\s*([0-9A-F]+)\s+([0-9A-F]+)\s+([0-9A-F]+)\s*\*/')
ins_re=re.compile(r'/\*.*?\*/\s+(\S+)\s*(.*)')
label_addr={}
instrs=[]
pending_labels=[]
for l in region:
    ls=l.strip()
    if ls.endswith(':') and not addr_re.search(ls):
        pending_labels.append(ls[:-1]); continue
    am=addr_re.search(l)
    if am:
        off=int(am.group(1),16); vram=int(am.group(2),16); word=am.group(3)
        im=ins_re.search(l)
        op=im.group(1); rest=im.group(2).split('/*')[0].strip()
        instrs.append({'off':off,'vram':vram,'word':word,'op':op,'rest':rest,
                       'labels':pending_labels,'line':l.strip()})
        for p in pending_labels: label_addr[p]=vram
        pending_labels=[]
for i,ins in enumerate(instrs):
    m=re.match(r'(func_\w+|\.[A-Za-z0-9_]+)',ins['rest'])
    ins['target']=m.group(1) if m else None
# block starts
starts=set()
for l,a in label_addr.items(): starts.add(a)
term={'b','beq','bne','beqz','bnez','bgez','bgtz','blez','bltz','bltzal','bgezal','j','jal','jr','jalr'}
for i,ins in enumerate(instrs):
    if ins["op"] in term and i+2<len(instrs):
        starts.add(instrs[i+2]["vram"])
starts.add(instrs[0]['vram'])
ss=sorted(starts)
blocks=[]
for bi,sv in enumerate(ss):
    idx=next(i for i,x in enumerate(instrs) if x['vram']==sv)
    ev=ss[bi+1] if bi+1<len(ss) else instrs[-1]['vram']+4
    body=[x for x in instrs if sv<=x['vram']<ev]
    blocks.append({'i':bi,'vram':sv,'off':body[0]['off'],'size':(ev-sv),
                   'n':len(body),'last':body[-1],'first':body[0],
                   'labels':body[0]['labels']})
# output
out=[]
for b in blocks:
    last=b['last']
    tgt=last['target'] if last['op'] in term else ''
    out.append({'i':b['i'],'off':'0x%05X'%b['off'],'vram':'0x%08X'%b['vram'],
                'size':'0x%X'%b['size'],'n':b['n'],
                'labels':b['labels'],
                'term':'%s %s'%(last['op'],last['rest']),
                'first':'%s %s'%(b['first']['op'],b['first']['rest'])})
json.dump(out,open('/tmp/blockmap.json','w'),indent=1)
print("blocks",len(blocks))
for b in out:
    lab=','.join(b['labels'])
    print("%3d %-8s %-10s sz=%-6s n=%-3d %-6s | %-28s | %s"%(b['i'],b['off'],b['vram'],b['size'],b['n'],lab[:16],b['first'][:28],b['term'][:40]))
