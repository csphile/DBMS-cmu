select work.name, work_type.name
from work 
	inner join (
		select max(length(work.name)) as max_len, 
			work.type as type 
		from work group by type
	) as newtable on newtable.max_len = length(work.name) 
		and work.type = newtable.type 
	inner join work_type on work_type.id = work.type
order by work_type.name asc, 
		work.name asc; 
