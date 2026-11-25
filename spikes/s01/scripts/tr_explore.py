import sys; sys.path.insert(0,'scripts')
from common import *
import collections
t=load('data/transition.csv')
g=collections.defaultdict(list)
for r in t:
    reg='chaos' if r['chaos_db']>-20 else 'reg'
    g[(reg,r['curve'],int(r['R']))].append((r['burst_db'],r['ctl_burst_db'],r['jump']))
print('reg curve R n | burst p50 p95 p99 max | ctl p95 max | frac>1dB >3dB >6dB | jump p95')
for k in sorted(g):
    v=np.array(g[k]); b=v[:,0]
    print(k,len(v),'| %.2f %.2f %.2f %.2f | %.2f %.2f | %.3f %.3f %.3f | %.2f'%(np.median(b),np.percentile(b,95),np.percentile(b,99),b.max(),np.percentile(v[:,1],95),v[:,1].max(),(b>1).mean(),(b>3).mean(),(b>6).mean(),np.percentile(v[:,2],95)))
