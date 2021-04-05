select area.name, count(*) as c_id from artist
	inner join area on area.id = artist.area
where begin_date_year < 1850 
group by artist.area
order by c_id desc
limit 10;