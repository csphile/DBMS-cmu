select artist.name, new_table.c from artist 
	inner join ( select artist,
		count(distinct artist_alias.name) as c from artist_alias group by artist
		) as new_table
	on artist.id = new_table.artist
	inner join area
	on area.id = artist.area
where area == 221 and begin_date_year > 1950
order by c desc, 
	artist.name desc 
limit 10; 